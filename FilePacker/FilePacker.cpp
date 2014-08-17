#include "../src/pf_types.h"
#include <fileutils.h>
#include <string>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

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

vector<string> split(string const & str, char delim)
{
    vector<string> result;
    istringstream iss(str);
    string elem;
    while( getline(iss,elem,delim) ) {
        result.push_back(elem);
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
    die::NativeString filename;
    size_t filesize;
};
typedef vector<FileSpec> FileList;

void sortFileList(FileList & fileList)
{
    sort(fileList.begin(),fileList.end(),[](FileSpec const & f1, FileSpec const & f2) {
        return f1.filename < f2.filename;
    });
}

void doGetFileList(fs::Path path, vector<string> const & extensions, fs::Path base, FileList & fileList)
{
    for( auto ext : extensions ) {
        auto pathMask = path.append("*."+ext);
        for( fs::GlobIterator it(pathMask); it.getStatus() == fs::ok; ++it ) {
            fileList.push_back({
                fs::Path::changeSeparator(base.append(it->filename()),pf::separator),
                it->filesize()
            });
        }
    }    
}

FileList getFileList(fs::Path path, vector<string> const & extensions)
{
    FileList result;
    doGetFileList(path,extensions,""_dies,result);
    sortFileList(result);
    return result;
}

void doRecursiveFileList(fs::Path path, vector<string> const & extensions, fs::Path base, FileList & fileList)
{
    auto basePath = path.append(base);
    doGetFileList(basePath,extensions,base,fileList);
    // recurse subdirs
    for( auto it = fs::GlobIterator(basePath.append("*.*")); it.getStatus() == fs::ok; ++it ) {
        if( it->isDirectory() && ! it->isSpecialDirectory() ) {
            doRecursiveFileList(path,extensions,base.append(it->filename()),fileList);
        }
    }
}

FileList recursiveFileList(fs::Path path, vector<string> const & extensions)
{
    FileList result;
    doRecursiveFileList(path,extensions,""_dies,result);
    sortFileList(result);
    return result;
}

void writeEntries(FileList const & fileList)
{
    pf::FilePos pos = 0;
    
    for( auto const & f : fileList ) {
        wcout << 
                " filename " << f.filename <<
                " filesize " << f.filesize <<
                " pos " << pos <<
                endl;
        
        auto filename = f.filename.toUTF8();
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

void writeData(fs::Path path, FileList const & fileList)
{
    for( auto const & f : fileList ) {
        auto filename = path.append(f.filename).getPath();
        fs::FileStreamWrapper source(filename,ios_base::in|ios::binary);
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
    string extensionsS = parser.argument(3,"*");
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
    auto path = fs::Path(baseDir);
    auto extensions = split(extensionsS,',');
    
    FileList fileList = options.count("recursive") ? 
        recursiveFileList(path,extensions) :
        getFileList(path,extensions);
    total = fileList.size();
    writeEntries(fileList);
    writeData(path,fileList);    
    
    // write total 
    file.seekg(offset + sizeof(pf::HeaderSize) + sizeof(pf::currentVersion));
    write(file,total);  
    
    // write offset
    file.seekg(0,ios::end);
    write<pf::FilePos>(file,offset);

    return 0;
}

