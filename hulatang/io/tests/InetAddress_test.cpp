#include "hulatang/base/Log.hpp"
#include "hulatang/io/InetAddress.hpp"
#include <winsock2.h>

using hulatang::base::Log;
using hulatang::io::InetAddress;

int main(int _argc, const char **_argv)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    Log::init();
    InetAddress address{InetAddress::fromHostnameAndService("www.baidu.com", "https")};
    HLT_INFO("{}", address.addrLen());

    WSACleanup();
    return 0;
}