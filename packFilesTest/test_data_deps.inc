#include <cstdlib>
#include <string>
using namespace std::literals::string_literals;

#define xstr(a) str(a)
#define str(a) #a

namespace {

#ifdef BAZEL_FOR_WINDOWS
std::string baseDir = "../../../../../../"s;
#else
std::string baseDir;
#endif

std::string testDir = baseDir + "packFilesTest/"s;
std::string testDataDir = testDir + "testData/"s;

}
