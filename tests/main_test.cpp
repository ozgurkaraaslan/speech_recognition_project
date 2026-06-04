#include "CppUTest/CommandLineTestRunner.h"

int main(int ac, char** av)
{
    // Run CppUTest tests
    return CommandLineTestRunner::RunAllTests(ac, av);
}