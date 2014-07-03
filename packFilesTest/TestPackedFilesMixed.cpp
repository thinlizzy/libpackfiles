#include "../src/PackedFiles.h"
#include <tut.h>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>

#define BASETEST "testData/"

using namespace std;
using namespace pf;

namespace {
    struct setup {
        Files files;
		setup()
		{
            files = loadFromFile(BASETEST "test2.dat");
		}
    };
}

namespace tut { 
    typedef test_group<setup> tg;
    tg my_test_group("MixedPackedFiles");

	typedef tg::object testobject;

    template<> 
    template<> 
    void testobject::test<1>() 
    {
        set_test_name("loadFromFile count");
        
        ensure_equals(files.count(),4);
    }

    template<> 
    template<> 
    void testobject::test<2>() 
    {
        set_test_name("file2.txt integrity");
        
        auto res2 = files.find("file2.txt");
        ensure("file2.txt",res2);
        res2.seekg(0,ios::end);
        ensure_equals(res2.tellg(),5);
        
        res2.seekg(0,ios::beg);
        char buf1[5];
        res2.read(buf1,5);
        
        fstream file2(BASETEST "file2.txt",ios::in|ios::binary);
        char buf2[5];
        file2.read(buf2,5);
        
        ensure(equal(buf1,buf1+5,buf2));        
    }
    
    vector<char> readStream(istream & is, istream::pos_type size)
    {
        vector<char> buf(size);
        size_t p = 0;
        do {
            is.read(&buf[p],size);
            p += is.gcount();
        } while(is.gcount() > 0);

        return buf;
    }

    template<> 
    template<> 
    void testobject::test<3>() 
    {
        set_test_name("enable.ogg integrity");
        
        auto res = files.find("enable.ogg");
        ensure("enable.ogg",res);
        res.seekg(0,ios::end);
        auto size = res.tellg();
        ensure(size > 0);
        
        res.seekg(0);
        fstream file(BASETEST "enable.ogg",ios::in|ios::binary);
        
        auto buf1 = readStream(res,size);
        auto buf2 = readStream(file,size);
        
        ensure("buffers are different",equal(buf1.begin(),buf1.end(),buf2.begin()));        
    }
}
