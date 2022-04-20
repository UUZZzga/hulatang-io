#include "hulatang/base/Log.hpp"
#include "hulatang/io/InetAddress.hpp"

using hulatang::base::Log;
using hulatang::io::InetAddress;

int main(int _argc, const char **_argv)
{
    Log::init();
    InetAddress address{InetAddress::fromHostnameAndService("www.baidu.com", "https")};
    HLT_INFO("{}", address.sockaddrLength());
    HLT_INFO("{}", address.toString());
    return 0;
}