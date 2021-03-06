#include "PackedFiles.h"
#include <iostream>
#include <algorithm>

namespace pf {
    
template<typename T> T read(std::istream & is)
{
    T result;
    is.read(reinterpret_cast<char *>(&result),sizeof(T));
    return result;
}

int compare(Filename const & name, BStr const & bstr)
{
    return name.compare(0,name.length(),bstr.name.data(),bstr.size);
}

//
    
Files loadFromFile(ExternalFilename name)
{
    return Files(name);
}

Files loadFromExecutable(ExternalFilename name)
{
    // TODO placeholder to implementation specific executable fiddling. for now, it just behaves as an external file
    return loadFromFile(name);
}

// Files

Files::Files():
    numFiles(0)
{}

Files::Files(ExternalFilename name):
    bigFile(name,std::ios::in|std::ios::binary),
    numFiles(0)
{
    if( ! bigFile ) return;
        
    bigFile.seekg(-std::ifstream::off_type(sizeof(FilePos)),std::ios::end);
    auto posInFile = read<FilePos>(bigFile);
    bigFile.seekg(posInFile,std::ios::beg);

    // files start right after the header
    start = bigFile.tellg();
    start += read<HeaderSize>(bigFile);

    // TODO do logic based on version number
    auto version = read<Version>(bigFile);

    numFiles = read<FileIndex>(bigFile);
}

Resource Files::find(Filename name)
{
    return doFind(name,0,numFiles);
}

FileIndex Files::count() const
{
    return numFiles;
}

// TODO std::binary_search could be used if we supported iterators :>
Resource Files::doFind(Filename name, FileIndex first, FileIndex last)
{
    if( first >= last ) return Resource();
    
    auto elem = (first+last) / 2;
    bigFile.seekg(elemPos(elem));

    // we read each separate field to avoid alignment shenanigans
    FileEntry entry;
    entry.filename.size = read<NameSize>(bigFile);
    entry.filename.name = read<InternalName>(bigFile);
    entry.pos = read<FilePos>(bigFile);
    entry.size = read<FilePos>(bigFile);

    auto cmp = compare(name,entry.filename);
    if( cmp == 0 ) return Resource(bigFile,entry.pos + elemPos(numFiles),entry.size);
    
    return cmp < 0 ? doFind(name,first,elem) : doFind(name,elem+1,last);
}

std::streampos Files::elemPos(size_t elem) const
{
    return start + std::streampos(elem * pf::fileEntrySize);
}

}
