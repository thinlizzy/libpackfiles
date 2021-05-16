#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "src/pf_types.h"

using namespace std;

class CmdParser {
	int argc;
	char ** argv;
	unordered_multimap<string,string> flags;
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
		
		return unordered_set<string>(argv + num,argv + argc);
	}

	unordered_multimap<string,string> & flagsFrom(int num) {
		if( num >= argc ) return flags;

		string flagName;
		for_each(argv + num,argv + argc,[&flagName,this](char * arg) {
			if( arg[0] == '-' ) {
				flagName = string(arg).substr(1);
			} else {
				if( ! flagName.empty() ) {
					flags.insert({flagName,arg});
				}
			}
		});
		if( ! flagName.empty() && flags.count(flagName) == 0 ) {
			flags.insert({flagName,""});
		}
		return flags;
	}

	bool hasFlag(std::string const & flagName) {
		return flags.count(flagName);
	}

	static std::string const emptyFlag;

	std::string const & getFlagValue(std::string const & flagName) {
		auto it = flags.find(flagName);
		return it == flags.end() ? emptyFlag : it->second;
	}

	vector<std::string> getFlagValues(std::string const & flagName) {
		auto its = flags.equal_range(flagName);

		vector<string> result;
		for_each(its.first,its.second,[&result](auto const & pair) {
			result.push_back(pair.second);
		});
		return result;
	}
};

std::string const CmdParser::emptyFlag;

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
using FileList = vector<FileSpec>;

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
    for( auto && entry : std::filesystem::directory_iterator(path) ) {
        getFile(entry,extensions,result);
    }
    sortFileList(result);
    return result;
}

FileList recursiveFileList(std::filesystem::path path, Extensions const & extensions)
{
    FileList result;
    for( auto && entry : std::filesystem::recursive_directory_iterator(path) ) {
        getFile(entry,extensions,result);
    }
    sortFileList(result);
    return result;
}

FileList getFileListFromFiles(vector<string> const & filenames) {
    FileList result;
    for( auto && filename : filenames ) {
		auto entry = std::filesystem::directory_entry(filename);
        result.push_back({
            entry.path(),
            entry.file_size()
        });
    }
    sortFileList(result);
    return result;
}

std::filesystem::path commonBasePath(std::filesystem::path const & p1, std::filesystem::path const & p2) {
	std::filesystem::path result;
	auto its = mismatch(p1.begin(),p1.end(),p2.begin(),p2.end());
	for_each(p1.begin(),its.first,[&result](auto p) {
		result /= p;
	});
	return result;
}

std::filesystem::path getCommonBaseDir(FileList const & fileList) {
	std::filesystem::path result;
	if( fileList.empty() ) return result; // should never happen
	result = fileList[0].filename;
	for_each(next(fileList.begin()),fileList.end(),[&result](auto & fileEntry) {
		result = commonBasePath(result,fileEntry.filename);
	});
	return result;
}

void writeEntries(std::filesystem::path path, FileList const & fileList)
{
    pf::FilePos pos = 0;
    
    for( auto const & f : fileList ) {
        cout <<
			" filename " << f.filename <<
			" filesize " << f.filesize <<
			" pos " << pos <<
			endl;

        auto filename = f.filename.lexically_relative(path).string();
        replace(filename.begin(),filename.end(),'\\',pf::separator); // replace win32 path separators if any
        if( filename.size() > pf::MaxInternalName ) {
            filename.resize(pf::MaxInternalName);
        }
        
        pf::FileEntry entry;
        entry.filename.size = filename.size();
        entry.filename.name.fill(255);
        copy(filename.begin(),filename.end(),begin(entry.filename.name));
        entry.pos = pos;
        entry.size = f.filesize;

        // we write each separate field to avoid alignment shenanigans
        write(file,entry.filename.size);
        write(file,entry.filename.name);
        write(file,entry.pos);
        write(file,entry.size);

        pos += entry.size;
    }
}

void writeData(FileList const & fileList)
{
    for( auto const & f : fileList ) {
        ifstream source(f.filename,ios_base::in | ios::binary);
        if( ! source ) {
            cerr << "Could not open " << f.filename << " for reading\n";
            exit(1);
        }
        char buffer[BUFSIZ];
        for(;;) {
            source.read(buffer,BUFSIZ);
            if( source.gcount() == 0 ) break;
            file.write(buffer,source.gcount());
        }
    }    
}

auto openFileAndGetOffset(string const & filename, bool append) {
    if( append ) {
        file.open(filename,ios_base::in | ios_base::out | ios_base::binary);
        if( ! file ) {
            cerr << "could not open " << filename << " for appending" << endl;
            exit(2);
        }
        
        file.seekg(0,ios::end);
    } else {
        file.open(filename,ios_base::trunc | ios_base::out | ios_base::binary);        
        if( ! file ) {
            cerr << "could not open " << filename << " for writing" << endl;
            exit(2);
        }
    }
    return file.tellg();
}

void writeHeader() {
    // header size
    pf::HeaderSize headerSize = 
        sizeof(pf::HeaderSize) + sizeof(pf::currentVersion)
        + sizeof(pf::FileIndex);
    write(file,headerSize);
    // version
    write(file,pf::currentVersion);
    // placeholder for total
    write(file,pf::FileIndex());
}

auto getFileListFromBaseAndExtensions(bool recursive, std::filesystem::path const & baseDir, Extensions const & extensions) {
    return recursive ? recursiveFileList(baseDir,extensions) : getFileList(baseDir,extensions);
}

void writeTotalAndOffset(pf::FileIndex total, fstream::pos_type offset) {
    // write total 
    file.seekg(offset + fstream::pos_type(sizeof(pf::HeaderSize)) + fstream::pos_type(sizeof(pf::currentVersion)));
    write(file,total);
    // write offset
    file.seekg(0,ios::end);
    write<pf::FilePos>(file,offset);
}

// // //

int main(int argc, char ** argv)
{
    CmdParser parser(argc,argv);
    
    string filename = parser.argument(1);
    auto & flags = parser.flagsFrom(2);
    if( flags.empty() ) {
        string baseDirS = parser.argument(2,"");
        string extensionsS = parser.argument(3,"");
        auto options = parser.optionsFrom(4);
        
        auto offset = openFileAndGetOffset(filename,options.count("append"));
        writeHeader();
        auto baseDir = std::filesystem::path(baseDirS);
        auto extensions = getExtensions(extensionsS,',');
        auto fileList = getFileListFromBaseAndExtensions(options.count("recursive"),baseDir,extensions);
        // write tables and files
        writeEntries(baseDir,fileList);
        writeData(fileList);
        writeTotalAndOffset(fileList.size(),offset);
    } else {
        if( flags.count("files") == 0 ) {
            cerr << "Flag -files is mandatory.\n";
            exit(1);
        }

        auto offset = openFileAndGetOffset(filename,parser.hasFlag("append"));
        writeHeader();
        auto fileList = getFileListFromFiles(parser.getFlagValues("files"));
        auto baseDir = parser.hasFlag("baseDir") 
			? std::filesystem::path(parser.getFlagValue("baseDir")) 
			: getCommonBaseDir(fileList);
        // write tables and files
        writeEntries(baseDir,fileList);
        writeData(fileList);
        writeTotalAndOffset(fileList.size(),offset);
    }

    return 0;
}
