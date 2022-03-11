#include "hulatang/http/URL.hpp"

#include <iostream>

using hulatang::http::URL;

void printURL(std::string_view urlStr)
{
    URL url(urlStr);
    std::cout << urlStr << std::endl
              << "protocol: " << url.protocol() << std::endl
              << "host: " << url.host() << std::endl
              << "port: " << url.port() << std::endl
              << "path: " << url.path() << std::endl
              << "query: " << url.query() << std::endl
              << "ref: " << url.ref() << std::endl
              << std::endl;
}

int main(int _argc, const char **_argv)
{
    printURL("http://www.baidu.com/index.html");
    printURL("http://www.baidu.com:80/index.html");
    printURL("http://www.baidu.com:80/index.html#1234");
    printURL("http://www.baidu.com:80/index?id=123");
    printURL("http://www.baidu.com:80/index?id=123#1234");
    printURL("http://[fe80::822a:a8ff:fe49:470c:tESt]:1234/keys");
    return 0;
}