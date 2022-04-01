#include "hulatang/file/LocalFile.hpp"

#include "hulatang/base/def.h"
#include "hulatang/io/EventLoop.hpp"
#include "hulatang/io/FdEventWatcher.hpp"
#include <algorithm>
#include <memory>
#include <system_error>

namespace hulatang::file {
LocalFile::LocalFile(io::EventLoop *loop) noexcept
    : loop_(loop)
    , writing_(false)
    , reading_(false)
    , readingAll_(false)
{}

LocalFile::~LocalFile() noexcept
{
    if (!watcher_.expired())
    {
        inLoopClose();
    }
}

void LocalFile::open(std::string_view path, base::OFlag mode) noexcept
{
    loop_->runInLoop([this, path, mode] { inLoopOpen(path, mode); });
}

void LocalFile::create(std::string_view path, base::OFlag mode) noexcept
{
    loop_->runInLoop([this, path, mode] { inLoopCreate(path, mode); });
}

void LocalFile::read()
{
    loop_->runInLoop([this] { inLoopReadAll(); });
}

void LocalFile::read(size_t size)
{
    loop_->runInLoop([this, size] { inLoopRead(size); });
}

void LocalFile::write(const char *buf, size_t size)
{
    loop_->runInLoop([this, buf = const_cast<char *>(buf), size] { inLoopWrite(buf, size); });
}

void LocalFile::close()
{
    loop_->runInLoop([this] { inLoopClose(); });
}

size_t LocalFile::tell()
{
    return pos_;
}

void LocalFile::seek(int64_t offset)
{
    loop_->runInLoop([this, offset] { seek(offset); });
}

void LocalFile::initFdChannel(const std::shared_ptr<io::FdEventWatcher> &watcher)
{
    loop_->getFdEventManager().add(watcher, fd_);
    using namespace std::placeholders;
    watcher->setReadHandler([this](auto &&PH1, auto &&PH2) { onRead(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
    watcher->setWriteHandler([this](auto &&PH1, auto &&PH2) { onWrite(std::forward<decltype(PH2)>(PH2)); });
    watcher->setCloseHandler([this] { loop_->runInLoop([this] { inLoopClose(); }); });
}

void LocalFile::inLoopOpen(std::string_view path, base::OFlag mode)
{
    loop_->assertInLoopThread();
    std::shared_ptr<io::FdEventWatcher> watcher(std::make_shared<io::FdEventWatcher>(loop_));
    watcher_ = watcher;

    std::error_condition condition;
    fd_.open(path, mode, condition);
    if (condition)
    {
        onError(condition);
        return;
    }

    initFdChannel(watcher);
    onOpen();
}

void LocalFile::inLoopCreate(std::string_view path, base::OFlag mode)
{
    loop_->assertInLoopThread();
    std::shared_ptr<io::FdEventWatcher> watcher(std::make_shared<io::FdEventWatcher>(loop_));
    watcher_ = watcher;

    std::error_condition condition;
    fd_.create(path, mode, condition);
    if (condition)
    {
        onError(condition);
        return;
    }
    initFdChannel(watcher);
    onOpen();
}

void LocalFile::inLoopClose()
{
    assert(!watcher_.expired());
    loop_->getFdEventManager().cancel(watcher_.lock(), fd_);
    fd_.close();
    watcher_.reset();
}

void LocalFile::inLoopReadAll()
{
    constexpr size_t readsize = 4096;
    inLoopRead(readsize);
    readingAll_ = true;
}

void LocalFile::inLoopRead(size_t size)
{
    std::error_condition ec;
    readBuffer_.ensureWritableBytes(size);
    fd_.read({readBuffer_.beginWrite(), size}, ec);
    if (ec)
    {
        onError(ec);
        return;
    }
    reading_ = true;
}

void LocalFile::inLoopWrite(char *data, size_t size)
{
    std::error_condition ec;
    fd_.write({data, size}, ec);
    if (ec)
    {
        onError(ec);
        return;
    }
    writing_ = true;
}

void LocalFile::inLoopSeek(int64_t offset)
{
    readBuffer_.retrieveAll();
    std::error_condition ec;
    fd_.lseek(offset, 0, ec);
    if (ec)
    {
        onError(ec);
        return;
    }
}

void LocalFile::onOpen()
{
    if (openCallback_)
    {
        openCallback_();
    }
}

void LocalFile::onRead(void *data, size_t size)
{
    assert(data == readBuffer_.beginWrite());
    readBuffer_.hasWritten(size);
    updatePosition(size);
    if (readingAll_ && size != 0)
    {
        inLoopReadAll();
    }
    else
    {
        reading_ = false;
        readingAll_ = false;

        if (readCallback_)
        {
            readCallback_(readBuffer_);
        }
    }
}

void LocalFile::onWrite(size_t size)
{
    writing_ = false;
    updatePosition(size);
    writeCallback_();
}

void LocalFile::onError(std::error_condition &ec)
{
    assert(ec);
    if (errorCallback_)
    {
        errorCallback_(ec);
    }
}

void LocalFile::updatePosition(size_t offset)
{
    pos_ += offset;
#if defined(HLT_PLATFORM_WINDOWS)
    fd_.updatePosition(pos_);
#endif
}

} // namespace hulatang::file
