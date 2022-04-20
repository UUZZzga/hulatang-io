#include "hulatang/base/def.h"
#include "hulatang/base/Exception.hpp"
#include <cstdio>

HLT_NOINLINE void test()
{
    throw hulatang::Exception("oops");
}

HLT_NOINLINE void foo()
{
    test();
}

int main(int _argc, const char **_argv)
{
    try
    {
        foo();
    }
    catch (const hulatang::Exception &e)
    {
        printf("reason: %s\n", e.what());
        printf("stack trace:\n\t%s\n", e.stackTrace());
    }
    return 0;
}