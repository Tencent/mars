#include <boost/filesystem.hpp>

int main(void)
{
    mars_boost::filesystem::copy_file("a", "b");
    return 0;
}
