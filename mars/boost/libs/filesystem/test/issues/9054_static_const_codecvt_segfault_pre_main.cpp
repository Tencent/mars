#include "boost/filesystem.hpp"

static const mars_boost::filesystem::path::codecvt_type& dummy =
    mars_boost::filesystem::path::codecvt();

int main()
{
    return 0;
}
