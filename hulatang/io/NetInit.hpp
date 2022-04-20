#include "hulatang/base/def.h"
#include <csignal>
#include <cstdio>

#if HLT_PLATFORM_WINDOWS
#    include <winsock2.h>
#else
#include <sys/signal.h>
#endif

namespace hulatang::io {
struct NetInit
{
    NetInit()
    {
#if HLT_PLATFORM_WINDOWS
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
        struct sigaction sa;
        sa.sa_flags = 0;
        sa.sa_handler = SIG_IGN;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGPIPE, &sa, nullptr);
#endif
    }

    ~NetInit()
    {
#if HLT_PLATFORM_WINDOWS
        WSACleanup();
#endif
    }

    static const NetInit& init(){
        static NetInit init;
        return init;
    };
};

} // namespace hulatang::io
