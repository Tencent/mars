#include <boost/filesystem.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <string>

using namespace mars_boost::filesystem;

//  The original bug report was that this broke:
//     path p(L"C:\\TEMP\\");
//     path r(p / "narrow");
//  That code now works, but ...

//  Nils Gladitz has provided this example ...

class Test
{
public:
    ~Test()
    {
        path p(L"C:\\TEMP\\");
        path r(p / "narrow");
    }
};

//  path p("narrow");

//  fails if static linked and Test object is global variable, but does not fail if
//  path p("narrow") line above is not commented out, and also does not fail if the
//  Test test2 line below is commented out.

Test test1;
Test test2;

int cpp_main(int, char*[])
{
    return 0;
}
