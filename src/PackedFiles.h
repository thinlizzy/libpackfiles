#ifndef PACKEDFILES_H_DIEGO_2014_06_28_14_46
#define	PACKEDFILES_H_DIEGO_2014_06_28_14_46

#include "pf_types.h"
#include "Resource.h"
#include <fstream>
#include <memory>

namespace pf {

// TODO \r\n is not being translated to \n (using binary mode on source stream)
class Files {
// factories    
    friend Files loadFromFile(ExternalFilename name);
    friend Files loadFromExecutable(ExternalFilename name);
public:
    Files();
    explicit Files(ExternalFilename name);
    Files(Files &&) = default;
    Files & operator=(Files &&) = default;
    Files(Files const &) = delete;
    Files & operator=(Files const &) = delete;
    
    Resource find(Filename name);
    // TODO add iteration? key filtering?
    FileIndex count() const;
private:
    // TODO remove unique_ptr once they decide to implement movable streams
    std::unique_ptr<std::fstream> bigFile;
    size_t numFiles;
    std::streampos start;
    
    Resource doFind(Filename name, FileIndex first, FileIndex last);
    std::streampos elemPos(size_t elem) const;
};

Files loadFromFile(ExternalFilename name);
Files loadFromExecutable(ExternalFilename name);

}

#endif
