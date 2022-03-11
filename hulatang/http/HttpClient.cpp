#include "hulatang/http/HttpClient.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/io/InetAddress.hpp"
#include "hulatang/base/Stream.hpp"

#include <memory>

constexpr size_t npos = std::string::npos;

namespace hulatang::http {
using io::InetAddress;

HttpClient::HttpClient(EventLoop *loop)
    : loop_(loop)
{}

void HttpClient::get(std::string_view urlStr, RequestCallback cb)
{
    URL url(urlStr);
    HttpRequest request;
    request.setPath(url.path());
    request.setVersion(HttpRequest::Version::kHttp11);
    request.setMethod(HttpRequest::Method::GET);
    request.addHeader("Host", url.host());
    request.addHeader("Accept", "application/json");
    // request.addHeader("Accept-Encoding", "gzip");
    // request.addHeader("Accept-Language", "en-US");
    get(url, request, cb);
}

void HttpClient::get(const URL &url, const HttpRequest &req, RequestCallback cb)
{
    assert(!client_);
    InetAddress address;
    if (!url.port().empty())
    {
        address = InetAddress::fromHostnameAndService(std::string{url.host()}, std::string{url.port()});
    }
    else
    {
        address = InetAddress::fromHostnameAndService(std::string{url.host()}, std::string{url.protocol()});
    }
    client_ = std::make_unique<TCPClient>(loop_, std::move(address));
    client_->setConnectionCallback([this](auto &&PH1) { onConnection(std::forward<decltype(PH1)>(PH1)); });
    client_->setMessageCallback(
        [this](auto &&PH1, auto &&PH2) { onMessage(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });

    base::OutputStream out(&writeBuffer_);
    out << req;
    HLT_TRACE("{}", writeBuffer_.toString_view());
    loop_->queueInLoop([this] { client_->connect(); });
}

void HttpClient::onConnection(const TCPConnectionPtr &conn)
{
    if (conn->isConnected())
    {
        readBuffer_.retrieveAll();
        conn->send({const_cast<char *>(writeBuffer_.data()), writeBuffer_.size()});
    }
}

void HttpClient::onMessage(const TCPConnectionPtr &conn, const base::Buf &buf)
{
    HLT_TRACE("{}", std::string_view{buf.buf, buf.len});
}

} // namespace hulatang::http