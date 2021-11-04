#include <cstdio>
#include "interpreter.hpp"

int main(int argc, char **argv)
{
        if (argc < 2) {
                fputs("Wrong amount of arguments\n", stderr);
                return 1;
        }
        Interpreter I;
        I.RunScript(argv[1]);
        return 0;
}

