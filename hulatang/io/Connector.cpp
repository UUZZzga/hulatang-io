#include "hulatang/io/Connector.hpp"

#include "hulatang/base/Log.hpp"
#include "hulatang/base/File.hpp"
#include <chrono>
#include <system_error>

#ifdef min
#    undef min
#endif // min

template<typename R, typename P>
std::chrono::duration<R, P> min(std::chrono::duration<R, P> lhs, std::chrono::duration<R, P> rhs)
{
    return lhs < rhs ? lhs : rhs;
}

namespace hulatang::io {
using std::chrono::operator""ms;
using std::chrono::operator""s;
static constexpr std::chrono::milliseconds kInitRetryDelayMs = 500ms;
static constexpr std::chrono::milliseconds kMaxRetryDelayMs = 30s;

Connector::Connector(EventLoop *_loop, InetAddress &_address)
    : loop(_loop)
    , address(_address)
    , retryDelay(kInitRetryDelayMs)
    , state(State::Disconnected)
{}

Connector::~Connector() = default;

void Connector::start()
{
    loop->runInLoop([_this = shared_from_this()] { _this->startInLoop(); });
}

void Connector::stop() {}

void Connector::connect()
{
    fd.socket(address.getSockaddr(), address.sockaddrLength());
#if HLT_PLATFORM_WINDOWS
    // windows ��ִ�� ConnectEx ǰҪִ��bind
    // ��Ȼ�� 10022
    union
    {
        sockaddr addr;
        base::socket::sockaddr_u au{};
    };
    addr.sa_family = address.getSockaddr()->sa_family;
    address.copyFromNative(&addr, address.sockaddrLength());
    fd.bind(&addr, address.sockaddrLength());
#endif
    watcher = std::make_shared<FdEventWatcher>(loop);
    loop->getFdEventManager().add(watcher, fd);
    std::error_condition ec;
    fd.connect(address.getSockaddr(), address.sockaddrLength(), ec);
    if (!ec)
    {
        connected();
        return;
    }
    assert(ec.category() == base::file_category());
    if (base::FileErrorCode::CONNECTING == ec.value())
    {
        connecting();
    }
    else if (base::FileErrorCode::DEADLINE_EXCEEDED == ec.value())
    {
        retry();
    }
    else
    {
        loop->getFdEventManager().cancel(watcher, fd);
        fd.close();
    }
}

void Connector::connecting()
{
    state = State::Connecting;
    watcher->setOpenHandler([_this = shared_from_this()] { _this->connected(); });
    watcher->setErrorHandler([_this = shared_from_this()](auto &ec) { _this->handleError(ec); });
}

void Connector::connected()
{
    state = State::Connected;
    newConnectionCallback(std::move(fd), watcher);
}

void Connector::retry()
{
    state = State::Disconnected;
    HLT_CORE_DEBUG("Connector::retry - Retry connecting to {} in {} milliseconds. ", address.toString(), retryDelay.count());
    loop->runAfter(retryDelay, [_this = shared_from_this()] { _this->startInLoop(); });
    retryDelay = ::min(retryDelay * 2, kMaxRetryDelayMs);
}

void Connector::startInLoop()
{
    loop->assertInLoopThread();
    assert(state == State::Disconnected);
    connect();
}

void Connector::handleError(std::error_condition &ec)
{
    HLT_CORE_ERROR("Connector::handleError state={}", static_cast<int>(state));
    HLT_CORE_TRACE("SO_ERROR = {}, message={}", ec.value(), ec.message());
    loop->getFdEventManager().cancel(watcher, fd);
    fd.close();
    retry();
}

} // namespace hulatang::io