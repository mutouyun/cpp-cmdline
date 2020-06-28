#include "cmdline.h"

int main(int argc, char * argv[]) {
    cmdline::parser a;
    a.push(cmdline::options
        {
            {
                "-h", "--help", "Print usage.", false, "",
                [](auto & a, auto &) {
                    a.print_usage();
                }
            },
            {
                "-t", "--test", "You must use this option.", true, "",
                [](auto &, auto &) { /*Do Nothing.*/ }
            },
            {
                "-o", "--output", "Print text.", true, "Hello World!",
                [](auto &, auto & str) {
                    std::cout << str << std::endl;
                }
            }
        });
    a.exec(argc, argv);
    return 0;
}
