#include <cstdlib>

#include "cmdline.h"

#define ASSERT_EQ(A, B) ((void)(((A) == (B)) || (std::abort(), 1)))

int main(int argc, char * argv[]) {
    cmdline::parser a;
    a.push({
            {
                "-h", "--help", "Print usage.", false, "",
                [](auto & a, auto &) {
                    a.print_usage();
                }
            }, {
                nullptr, "--test", "You must use this option.", true, "",
                [](auto &, auto &) { /*Do Nothing.*/ }
            }, {
                "-o", "--output", "Print text.", true, "Hello World!",
                [](auto &, auto & str) {
                    std::cout << str << std::endl;
                }
            }
        });
    ASSERT_EQ(a.exec(argc, argv), 0);
    return 0;
}
