#ifndef RESOURCE_H_DIEGO_2014_06_28_16_39
#define	RESOURCE_H_DIEGO_2014_06_28_16_39

#include <istream>
#include <memory>

namespace pf {
    
class ResourceBuf;

class Resource: public std::istream {
    // TODO remove unique_ptr once they decide to implement movable streams
    std::unique_ptr<ResourceBuf> streambuf;
public:
    Resource();
    Resource(Resource && other); // can't be default because streams can't move :(
    Resource & operator=(Resource && other);
    Resource(std::istream & source, std::streampos start, std::streampos size);
    ~Resource();
};

}

#endif
