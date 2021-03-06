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

// platform definition macros
#define HLT_PLATFORM_WINDOWS 0
#define HLT_PLATFORM_LINUX 0
#if defined(_WIN32) || defined(_WIN64)
#    undef HLT_PLATFORM_WINDOWS
#    define HLT_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#    undef HLT_PLATFORM_LINUX
#    define HLT_PLATFORM_LINUX 1
#else
#    error "Unknown host"
#endif

#if defined(__LP64__) || defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(__64BIT__) || defined(__mips64) ||      \
    defined(__powerpc64__) || defined(__ppc64__)
#    define HLT_ARCH_64 1
#    define HLT_ARCH 64
#else
#    define HLT_ARCH_32 1
#    define HLT_ARCH 32
#endif
#endif // HULATANG_BASE_DEF_H
