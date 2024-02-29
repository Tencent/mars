// Rob Conde <rob.conde@ai-solutions.com> reports this fails
// to compile for Boost 1.58 with g++ 4.4.7 but is OK with FC++ 2013

#include "boost/filesystem/operations.hpp"

void myFunc()
{
    using namespace mars_boost::filesystem;

    copy_options opt(copy_options::overwrite_existing);

    copy_file(path("p1"), path("p2"), copy_options::overwrite_existing);
    //   copy_file(path("p1"),path("p2"),opt);
}