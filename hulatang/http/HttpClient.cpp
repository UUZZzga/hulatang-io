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
    request.setVersion(Version::Http11);
    request.setMethod(HttpRequest::Method::GET);
    request.addHeader("Host", url.host());
    request.addHeader("Accept", "text/html");
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
    HLT_CORE_TRACE("{}", writeBuffer_.toString_view());
    requestCallback_ = cb;
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
    readBuffer_.append(buf.buf, buf.len);
    HLT_CORE_TRACE("{}", readBuffer_.toString_view());
    std::error_condition ec;
    HttpResponse response = HttpResponse::buildFromBuffer(readBuffer_, ec);
    if (ec)
    {
        if (ec.value() != HttpResponse::WaitForData)
        {
            HLT_CORE_WARN("HttpClient error: {}", ec.value());
        }
        return;
    }
    requestCallback_(response);
}

} // namespace hulatang::http