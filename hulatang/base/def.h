#ifndef HULATANG_BASE_DEF_H
#define HULATANG_BASE_DEF_H

// Compilers macros
// https://sourceforge.net/p/predef/wiki/Compilers/
#if defined(__clang__)
// clang defines __GNUC__ or _MSC_VER
#    undef HLT_COMPILER_CLANG
#    define HLT_COMPILER_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#    if defined(__clang_analyzer__)
#        undef HLT_COMPILER_CLANG_ANALYZER
#        define HLT_COMPILER_CLANG_ANALYZER 1
#    endif // defined(__clang_analyzer__)
#elif defined(_MSC_VER)
#    undef HLT_COMPILER_MSVC
#    define HLT_COMPILER_MSVC _MSC_VER
#elif defined(__GNUC__)
#    undef HLT_COMPILER_GCC
#    define HLT_COMPILER_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#    error "HLT_COMPILER_* is not defined!"
#endif //

#ifdef _MSC_VER
#    define HLT_NOINLINE __declspec(noinline)
#else // ^^^ _MSC_VER ^^^ | vvv !_MSC_VER vvv
#    define HLT_NOINLINE __attribute__((noinline))
#endif //  !_MSC_VER

#endif // HULATANG_BASE_DEF_H
