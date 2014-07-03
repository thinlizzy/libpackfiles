#include "../src/PackedFiles.h"
#include <tut.h>
#include <string>

#define BASETEST "testData/"

using namespace std;
using namespace pf;

namespace {
    struct setup {
        Files textFiles;
		setup():
            textFiles(BASETEST "test3.dat")
		{
		}
    };
}

namespace tut { 
    typedef test_group<setup> tg;
    tg my_test_group("RecursiveTextPackedFiles");

	typedef tg::object testobject;

    template<> 
    template<> 
    void testobject::test<1>() 
    {
        set_test_name("loadFromFile count");
        
        ensure_equals(textFiles.count(),9);
    }
    
    template<> 
    template<> 
    void testobject::test<2>() 
    {
        set_test_name("loadFromFile find root");
        
        auto res1 = textFiles.find("file1.txt");
        ensure("1",res1);
        auto res2 = textFiles.find("file2.txt");
        ensure("2",res2);
        auto res3 = textFiles.find("file3.txt");
        ensure("3",res3);
        auto res4 = textFiles.find("file22.txt");
        ensure_not("4",res4);
        auto res5 = textFiles.find("file0.txt");
        ensure_not("5",res5);
    }

    template<> 
    template<> 
    void testobject::test<3>() 
    {
        set_test_name("loadFromFile find other dirs");
        
        ensure("1",textFiles.find("more/file1.txt"));
        ensure("2",textFiles.find("more/file2.txt"));
        ensure("3",textFiles.find("more2/file2.txt"));
        ensure("4",textFiles.find("more2/file3.txt"));
    }

    template<> 
    template<> 
    void testobject::test<4>() 
    {
        set_test_name("loadFromFile read from other dirs");
        
        auto res1 = textFiles.find("more/1/file1.txt");
        ensure("1",res1);
        string test;
        getline(res1,test);
        ensure_equals(test,"DIEGO");
    }
}