#include <tut.h>
#include <fstream>
#include <string>

#include "../src/Resource.h"

using namespace std;
using namespace pf;

namespace {
    struct setup {
        fstream mainStream;
		setup()
		{
            mainStream.open("packFilesTest.cpp");
		}
    };
}

namespace tut { 
    typedef test_group<setup> tg;
    tg resource_test_group("Resource");

	typedef tg::object testobject;

    template<> 
    template<> 
    void testobject::test<1>() 
    {
        set_test_name("get");

        Resource res(mainStream,9,3);
        ensure_equals(res.get(),'<');
        ensure_equals(res.get(),'t');
        ensure_equals(res.get(),'u');
        ensure_equals(res.get(),Resource::traits_type::eof());
    }   

    template<> 
    template<> 
    void testobject::test<2>() 
    {
        set_test_name("getline");

        Resource res(mainStream,9,7);
        string test;
        getline(res,test);
        ensure_equals(test,"<tut.h>");
    }   

    template<> 
    template<> 
    void testobject::test<3>() 
    {
        set_test_name("multiple getlines");

        Resource res(mainStream,9,37);
        string test;
        getline(res,test);
        ensure_equals(test,"<tut.h>");
        getline(res,test,' ');
        ensure_equals(test,"#include");
        getline(res,test,'\n');
        ensure_equals(test,"<tut_reporter.h>");
    }   

    template<> 
    template<> 
    void testobject::test<4>() 
    {
        set_test_name("read");

        Resource res(mainStream,9,7);
        string test;
        test.resize(10);
        res.read(&test[0],10);
        ensure_equals(res.gcount(),7);
        test.resize(res.gcount());
        ensure_equals(test,"<tut.h>");
    }   

    template<> 
    template<> 
    void testobject::test<5>() 
    {
        set_test_name("operator >>");

        Resource res(mainStream,1,16);
        string test;
        res >> test;
        ensure_equals(test,"include");
        res >> test;
        ensure_equals(test,"<tut.h>");
        res >> test;
        ensure(res.fail());
    }   

    template<> 
    template<> 
    void testobject::test<6>() 
    {
        set_test_name("multiple resources");

        Resource res1(mainStream,1,7);
        Resource res2(mainStream,18,20);
        Resource res3(mainStream,9,3);
        string test;
        res1 >> test;
        ensure_equals(test,"include");

        test.resize(10);
        res2.read(&test[0],10);
        ensure_equals(res2.gcount(),10);

        res3 >> test;
        ensure_equals(test,"<tu");

        res1 >> test;
        ensure(res1.fail());

        res3 >> test;
        ensure(res3.fail());

        test.resize(10);
        res2.read(&test[0],10);
        ensure_equals(res2.gcount(),10);
        
        res2.read(&test[0],10);
        ensure_equals(res2.gcount(),0);
        ensure(res2.fail());
    }   
    
    template<> 
    template<> 
    void testobject::test<7>() 
    {
        set_test_name("seek");

        Resource res(mainStream,9,37);
        res.seekg(8);
        ensure("seekg failed",res);
        string test;
        getline(res,test,' ');
        ensure_equals(test,"#include");
    }

    template<> 
    template<> 
    void testobject::test<8>() 
    {
        set_test_name("seek cur");

        Resource res(mainStream,9,37);
        string test;
        getline(res,test);
        ensure_equals(test,"<tut.h>");
        res.seekg(-7,ios::cur);
        ensure("seekg failed",res);
        getline(res,test,'.');
        ensure_equals(test,"tut");
        res.seekg(3,ios::cur);
        getline(res,test,' ');
        ensure_equals(test,"#include");
    }

    template<> 
    template<> 
    void testobject::test<9>() 
    {
        set_test_name("seek end");

        Resource res(mainStream,9,37);
        string test;
        getline(res,test);
        ensure_equals(test,"<tut.h>");
        res.seekg(-7,ios::end);
        ensure("seekg failed",res);
        getline(res,test);
        ensure_equals(test,".h>");
    }

}

