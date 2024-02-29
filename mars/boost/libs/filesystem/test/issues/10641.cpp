#include <iostream>
#include <boost/filesystem/path.hpp>
using namespace std;
namespace fs = mars_boost::filesystem;
int main(int argc, char** argv)
{

    try
    {
        fs::path my_path("test/test.txt");
        cout << "current path is      " << my_path << endl;
        cout << "parent path is       " << my_path.parent_path() << endl;
    }
    catch (std::exception& e)
    {
        cerr << endl
             << "Error during execution: " << e.what() << endl
             << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
