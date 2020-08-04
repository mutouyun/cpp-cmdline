#include <cstdlib>
#include <sstream>

#include "cmdline.h"

#define ASSERT_EQ(A, B) ((void)(((A) == (B)) || (std::abort(), 1)))

int main(int argc, char * argv[]) {
    cmdline::parser a;
    a.push({
            {
                "-h", "--help", "Print usage.", false, "",
                [&a](auto &) {
                    a.print_usage();
                }
            }, {
                nullptr, "--test", "You must use this option.", true, "",
                [](auto &) { /*Do Nothing.*/ }
            }, {
                "-o", "--output", "Print text.", true, "Hello World!",
                [&a](auto & str) {
                    a.print(str, "\n");
                }
            }
        });
    ASSERT_EQ(a.exec(argc, argv), 0);

    std::ostringstream ss;
    char const * args[] = {
        argv[0],
        "--test",
        "-o=whatever"
    };
    a.set_printer(ss);
    ASSERT_EQ(a.exec(3, args), 0);
    ASSERT_EQ(ss.str(), "whatever\n");

    return 0;
}
