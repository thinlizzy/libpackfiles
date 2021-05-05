#include <tut.h>
#include <string>
#include "src/PackedFiles.h"

#include "test_data_deps.inc"

using namespace std;
using namespace pf;

namespace {
    struct setup {
        Files textFiles;
		setup():
            textFiles(testDataDir + "test.dat")
		{
		}
    };
}

namespace tut { 
    typedef test_group<setup> tg;
    tg packedFiles_test_group("TextPackedFiles");

	typedef tg::object testobject;

    template<> 
    template<> 
    void testobject::test<1>() 
    {
        set_test_name("loadFromFile count");
        
        ensure_equals(textFiles.count(),3);
    }

    template<> 
    template<> 
    void testobject::test<2>() 
    {
        set_test_name("loadFromFile find");
        
        auto res1 = textFiles.find("file1.txt");
        ensure("1",!res1.fail());
        auto res2 = textFiles.find("file2.txt");
        ensure("2",!res2.fail());
        auto res3 = textFiles.find("file3.txt");
        ensure("3",!res3.fail());
        auto res4 = textFiles.find("file22.txt");
        ensure_not("4",!res4.fail());
        auto res5 = textFiles.find("file0.txt");
        ensure_not("5",!res5.fail());
    }

    template<> 
    template<> 
    void testobject::test<3>() 
    {
        set_test_name("loadFromFile file1");

        string test;
        auto res = textFiles.find("file1.txt");
        getline(res,test);
        ensure("ok",!res.fail());
        ensure_equals(test,"LINE1");
        getline(res,test);
        ensure_equals(test,"=====");
    }

    template<> 
    template<> 
    void testobject::test<4>() 
    {
        set_test_name("loadFromFile file2");

        string test;
        auto res = textFiles.find("file2.txt");
        getline(res,test);
        ensure("ok",!res.fail());
        ensure_equals(test,"LINE1");
        getline(res,test);
        ensure("fail",res.fail());
    }

    template<> 
    template<> 
    void testobject::test<5>() 
    {
        set_test_name("loadFromFile file2");

        string test;
        auto res = textFiles.find("file2.txt");
        getline(res,test);
        ensure("ok",!res.fail());
        ensure_equals(test,"LINE1");
        getline(res,test);
        ensure("fail",res.fail());
    }

    template<> 
    template<> 
    void testobject::test<6>() 
    {
        set_test_name("loadFromFile file3");

        string test;
        auto res = textFiles.find("file3.txt");
        getline(res,test);
        ensure_equals(test,"LINE1");
        getline(res,test);
        ensure_equals(test,"");
        getline(res,test);
        ensure_equals(test,"LINE2");
        getline(res,test);
        ensure_equals(test,"");
        getline(res,test);
        ensure_equals(test,"LINE3");
        getline(res,test);
        ensure_equals(test,"");
        getline(res,test);        
        ensure("fail",res.fail());
        
        res = move(textFiles.find("file3.txt"));
        getline(res,test);
        ensure_equals(test,"LINE1");
    }
}
