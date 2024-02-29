#include <boost/filesystem.hpp>
#include <cvt/cp950>
#include <iostream>
#include <string>
#include <locale>

namespace fs = mars_boost::filesystem;

int main(void)
{

    std::locale global_loc = std::locale();
    std::locale loc(global_loc, new stdext::cvt::codecvt_cp950< wchar_t >);
    fs::path::imbue(loc);

    std::cout << "HEADS UP! PIPE OUTPUT TO FILE AND INSPECT WITH HEX OR CP950 EDITOR.\n"
                 "WINDOWS COMMAND PROMPT FONTS DON'T SUPPORT CHINESE,\n"
                 "EVEN WITH CODEPAGE SET AND EVEN AS OF WIN 10 TECH PREVIEW."
              << std::endl;

    fs::recursive_directory_iterator end;
    fs::recursive_directory_iterator iter("C:/boost/test-files/utf-8");

    while (iter != end)
    {
        if (fs::is_directory(*iter))
        {
            std::cout << "[directory] " << iter->path().generic_string() << std::endl;
        }
        else if (fs::is_regular(*iter))
        {
            std::cout << " [file] " << iter->path().generic_string() << std::endl;
        }
        ++iter;
    }
    return 0;
}
