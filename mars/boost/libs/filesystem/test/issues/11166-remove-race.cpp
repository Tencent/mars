#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <fstream>

mars_boost::condition_variable cond;
mars_boost::mutex mut;

#define FNAME ("remove-test")
void remover()
{
    while (1)
    {
        mars_boost::filesystem::remove(FNAME);
    }
}

void creater()
{
    for (int i = 0; i < 100000; i++)
        std::fstream(FNAME, std::fstream::out);
}

int main()
{
    mars_boost::filesystem::remove(FNAME);
    mars_boost::filesystem::remove(FNAME);

    std::cout << "If you got this far, it's OK to remove a file that doesn't exist\n"
                 "Now trying with one creator thread and two remover threads.\n"
                 "This is likely to crash after just a few seconds at most."
              << std::endl;

    mars_boost::thread c(creater), r1(remover), r2(remover);

    c.join();
    r1.interrupt();
    r1.join();
    r2.interrupt();
    r2.join();
}
