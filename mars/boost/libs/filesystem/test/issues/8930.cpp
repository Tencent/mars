// Before running this test: export LANG=foo

#include <boost/filesystem.hpp>
int main()
{
    mars_boost::filesystem::path("/abc").root_directory();
}
