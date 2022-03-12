#ifndef HULATANG_HTTP_HTTPCLIENT_HPP
#define HULATANG_HTTP_HTTPCLIENT_HPP

#include "hulatang/base/Buffer.hpp"
#include "hulatang/http/HttpRequest.hpp"
#include "hulatang/http/HttpResponse.hpp"
#include "hulatang/http/URL.hpp"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/TCPClient.hpp"

namespace hulatang::http {
class HttpClient
{
public:
    using EventLoop = io::EventLoop;
    using TCPClient = io::TCPClient;
    using TCPConnectionPtr = io::TCPConnectionPtr;
    using RequestCallback = std::function<void(const HttpResponse &)>;

    explicit HttpClient(EventLoop *loop);

    void get(std::string_view urlStr, RequestCallback cb);

    void get(std::string_view url, const HttpRequest &req, RequestCallback cb)
    {
        get(URL{url}, req, cb);
    }

    void get(const URL &url, const HttpRequest &req, RequestCallback cb);

private:
    void onConnection(const TCPConnectionPtr &conn);
    void onMessage(const TCPConnectionPtr &conn, const base::Buf &buf);

    EventLoop *loop_;
    std::unique_ptr<TCPClient> client_;
    base::Buffer readBuffer_;
    base::Buffer writeBuffer_;
    RequestCallback requestCallback_;
};
} // namespace hulatang::http

#endif // HULATANG_HTTP_HTTPCLIENT_HPP
