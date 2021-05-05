#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include "src/pf_types.h"

using namespace std;

class CmdParser {
    int argc;
    char ** argv;
public:
    CmdParser(int argc, char ** argv):
        argc(argc),
        argv(argv)
    {}
    
    char const * argument(int num) {
        if( num >= argc ) {
            cout << "Usage FilePacker filename [basedir] [ext,ext...] [recursive append]" << endl <<
                "basedir defaults to the current dir. ext defaults to *" << endl <<
                "append - appends the packedfile into filename (usually an executable file)" << endl <<
                "recursive - traverses to all subdirs to find files with the specified extensions" << endl;
            exit(1);
        }
        
        return argv[num];
    }
    
    char const * argument(int num, char const * def) {
        if( num >= argc ) return def;
        
        return argv[num];
    }
    
    unordered_set<string> optionsFrom(int num) {
        if( num >= argc ) return unordered_set<string>();
        
        return unordered_set<string>(argv + num, argv + argc);
    }
};

using Extensions = unordered_set<std::string>;

Extensions getExtensions(string const & str, char delim)
{
    Extensions result;
    istringstream iss(str);
    string elem;
    while( getline(iss,elem,delim) ) {
        result.insert("." + elem);
    }
    return result;
}

template<typename T>
void write(ostream & os, T value)
{
    os.write(reinterpret_cast<char const *>(&value),sizeof(T));
}

fstream file;
struct FileSpec {
    std::filesystem::path filename;
    size_t filesize;
};
typedef vector<FileSpec> FileList;

void sortFileList(FileList & fileList)
{
    sort(fileList.begin(),fileList.end(),[](FileSpec const & f1, FileSpec const & f2) {
        return f1.filename < f2.filename;
    });
}

void getFile(std::filesystem::directory_entry const & entry, Extensions const & extensions, FileList & fileList)
{
    if( entry.is_regular_file() && (extensions.empty() || extensions.count(entry.path().extension().string())) ) {
        fileList.push_back({
            entry.path(),
            entry.file_size()
        });
    }
}

FileList getFileList(std::filesystem::path path, Extensions const & extensions)
{
    FileList result;
    for( auto & entry : std::filesystem::directory_iterator(path) ) {
        getFile(entry,extensions,result);
    }
    sortFileList(result);
    return result;
}

FileList recursiveFileList(std::filesystem::path path, Extensions const & extensions)
{
    FileList result;
    for( auto & entry : std::filesystem::recursive_directory_iterator(path) ) {
        getFile(entry,extensions,result);
    }
    sortFileList(result);
    return result;
}

void writeEntries(FileList const & fileList)
{
    pf::FilePos pos = 0;
    
    for( auto const & f : fileList ) {
        cout <<
			" filename " << f.filename <<
			" filesize " << f.filesize <<
			" pos " << pos <<
			endl;
        
        auto filename = f.filename.string();
        if( filename.size() > pf::MaxInternalName ) {
            filename.resize(pf::MaxInternalName);
        }
        
        pf::FileEntry entry;
        entry.filename.size = filename.size();
        copy(filename.begin(),filename.end(),begin(entry.filename.name));
        entry.pos = pos;
        entry.size = f.filesize;
        write(file,entry);
        pos += entry.size;
    }
}

void writeData(std::filesystem::path path, FileList const & fileList)
{
    for( auto const & f : fileList ) {
        auto filename = path;
        filename += f.filename;
        std::ifstream source(filename,ios_base::in|ios::binary);
        char buffer[BUFSIZ];
        for(;;) {
            source.read(buffer,BUFSIZ);
            if( source.gcount() == 0 ) break;
            file.write(buffer,source.gcount());
        }
    }    
}

// // //

int main(int argc, char ** argv)
{
    CmdParser parser(argc,argv);
    
    string filename = parser.argument(1);
    string baseDir = parser.argument(2,"");
    string extensionsS = parser.argument(3,"");
    auto options = parser.optionsFrom(4);
    
    fstream::pos_type offset;
    if( options.count("append") ) {
        file.open(filename,ios_base::in | ios_base::out | ios_base::binary);
        if( ! file ) {
            cout << "could not open the file for appending" << endl;
            exit(2);            
        }
        
        file.seekg(0,ios::end);
        offset = file.tellg();
    } else {
        file.open(filename,ios_base::trunc | ios_base::out | ios_base::binary);        
        if( ! file ) {
            cout << "could not open the file for writing" << endl;
            exit(2);            
        }
        
        offset = 0;
    }
    
    // placeholders
    // header size
    pf::HeaderSize headerSize = 
            sizeof(pf::HeaderSize) + sizeof(pf::currentVersion)
            + sizeof(pf::FileIndex);
    write(file,headerSize);
    // version
    write(file,pf::currentVersion);
    // total
    pf::FileIndex total = 0;
    write(file,total);
        
    // write tables and files
    auto path = std::filesystem::path(baseDir);
    auto extensions = getExtensions(extensionsS,',');
    
    FileList fileList = options.count("recursive") ? 
        recursiveFileList(path,extensions) :
        getFileList(path,extensions);
    total = fileList.size();
    writeEntries(fileList);
    writeData(path,fileList);    
    
    // write total 
    file.seekg(offset + fstream::pos_type(sizeof(pf::HeaderSize)) + fstream::pos_type(sizeof(pf::currentVersion)));
    write(file,total);  
    
    // write offset
    file.seekg(0,ios::end);
    write<pf::FilePos>(file,offset);

    return 0;
}
