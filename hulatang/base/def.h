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

#if HLT_COMPILER_MSVC
#    define HLT_NOINLINE __declspec(noinline)
#else // ^^^ HLT_COMPILER_MSVC ^^^ | vvv !HLT_COMPILER_MSVC vvv
#    define HLT_NOINLINE __attribute__((noinline))
#endif //  !HLT_COMPILER_MSVC

// function definition macros
#if HLT_COMPILER_MSVC
#    define HLT_FUNC_DEF __FUNCSIG__
#else
#    define HLT_FUNC_DEF __PRETTY_FUNCTION__
#endif

#endif // HULATANG_BASE_DEF_H
