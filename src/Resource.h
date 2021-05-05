#ifndef RESOURCE_H_DIEGO_2014_06_28_16_39
#define	RESOURCE_H_DIEGO_2014_06_28_16_39

#include <istream>
#include <memory>

namespace pf {
    
class ResourceBuf;

class Resource: public std::istream {
    // TODO remove unique_ptr after figuring out how to make ResourceBuf to move properly
    std::unique_ptr<ResourceBuf> streambuf;
public:
    Resource();
    Resource(Resource && other);
    Resource & operator=(Resource && other);
    Resource(std::istream & source, std::streampos start, std::streampos size);
    ~Resource();
    
    // function used to emulate resources from entire files
    // it leaves source stream at its end
    static Resource wrapStream(std::istream & source);
};

}

#endif
