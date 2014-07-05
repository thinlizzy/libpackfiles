#include "Resource.h"

namespace pf {
    
class ResourceBuf: public std::streambuf {
    std::istream & source;
    std::streampos start;
    std::streampos end;
    std::streampos pos;
public:
    ResourceBuf(std::istream & source, std::streampos start, std::streampos size):
        source(source),
        start(start),
        end(start+size),
        pos(start)
    {}
        
    std::streamsize showmanyc() override { return end-pos; }
    
    auto underflow() -> traits_type::int_type override {
        if( showmanyc() <= 0 ) return traits_type::eof();
        
        source.seekg(pos);
        return source.get();
    }
    
    auto uflow() -> traits_type::int_type override {
        auto result = underflow();
        pos += 1;
        return result;
    }
    
    std::streamsize xsgetn(char * s, std::streamsize n) override {
        auto toRead = std::min(n,showmanyc());
        if( toRead == 0 ) return 0;
        
        source.seekg(pos);
		if( ! source.read(s,toRead) ) return traits_type::eof();

		pos += source.gcount();

        return source.gcount();
    }
    
    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override {
        std::streampos newPos;
        switch(dir) {
            case std::ios::end:
                newPos = end+off;
                break;
            case std::ios::cur:
                newPos = pos+off;
                break;                
            case std::ios::beg:
            default:
                newPos = start+off;
                break;                                
        }
        
        return doSeekpos(newPos,which);
    }
        
    pos_type seekpos(pos_type pos, std::ios_base::openmode which) override {
        return doSeekpos(start+pos,which);        
    }
    
    pos_type doSeekpos(pos_type newPos, std::ios_base::openmode which) {
        auto failurePos = pos_type(off_type(-1));
        if( (which & std::ios_base::in) == 0 ) return failurePos;
        
        if( newPos < start || newPos > end ) return failurePos;
        if( ! source.seekg(newPos) ) return failurePos;
        
        pos = newPos;
        return pos-start;
    }
};
    
Resource::Resource():
	std::istream(0)
{}

Resource::Resource(std::istream & source, std::streampos start, std::streampos size):
	std::istream(0),
    streambuf(new ResourceBuf(source,start,size))
{
    rdbuf(streambuf.get());
}

Resource::Resource(Resource && other):
	std::istream(0)
{
    other.rdbuf(0);
    streambuf = std::move(other.streambuf);
    rdbuf(streambuf.get());
}

Resource & Resource::operator=(Resource && other)
{
    other.rdbuf(0);
    streambuf = std::move(other.streambuf);
    rdbuf(streambuf.get());
    return *this;
}

Resource::~Resource()
{   
}

Resource Resource::wrapStream(std::istream & source)
{
    source.seekg(0,std::ios::end);
    auto last = source.tellg();
    return Resource(source,0,last);
}

}