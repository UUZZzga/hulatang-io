#ifndef HULATANG_FILE_LOCALFILE_HPP
#define HULATANG_FILE_LOCALFILE_HPP

#include "hulatang/base/File.hpp"
#include "hulatang/base/Buffer.hpp"
#include "hulatang/io/FdEventWatcher.hpp"

#include <atomic>
#include <functional>
#include <string_view>
#include <system_error>

namespace hulatang::io {
class EventLoop;
}

namespace hulatang::file {
class LocalFile
{
public:
    using BufferCallBack = std::function<void (base::Buffer &)>;
    using CallBack = std::function<void()>;
    using ErrorCallBack = std::function<void(std::error_condition &)>;

public:
    explicit LocalFile(hulatang::io::EventLoop *loop) noexcept;
    ~LocalFile() noexcept;

    void open(std::string_view path, base::OFlag mode) noexcept;
    void create(std::string_view path, base::OFlag mode) noexcept;

    void read();
    void read(size_t size);

    void write(const char *buf, size_t size);
    void write(std::string_view str);
    void write(void *buffer);

    void close();

    size_t tell();
    void seek(int64_t offset);

    void setOpenCallback(const CallBack &openCallback) { openCallback_ = openCallback; }

    void setWriteCallback(const CallBack &writeCallback) { writeCallback_ = writeCallback; }

    void setReadCallback(const BufferCallBack &readCallback) { readCallback_ = readCallback; }

    void setErrorCallback(const ErrorCallBack &errorCallback) { errorCallback_ = errorCallback; }

private:
    void initFdChannel(const std::shared_ptr<io::FdEventWatcher> &watcher);
    void inLoopOpen(std::string_view path, base::OFlag mode);
    void inLoopCreate(std::string_view path, base::OFlag mode);
    void inLoopClose();
    void inLoopReadAll();
    void inLoopRead(size_t size);
    void inLoopWrite(char *data, size_t size);
    void inLoopSeek(int64_t offset);

    void onOpen();
    void onRead(void *data, size_t size);
    void onWrite(size_t size);
    void onError(std::error_condition &ec);

    void updatePosition(size_t offset);

private:
    CallBack openCallback_;
    CallBack writeCallback_;
    BufferCallBack readCallback_;
    ErrorCallBack errorCallback_;
    io::EventLoop *loop_;
    std::weak_ptr<io::FdEventWatcher> watcher_;
    base::FileDescriptor fd_;
    std::atomic<uint64_t> pos_;
    base::Buffer readBuffer_;
    bool writing_;
    bool reading_;
    bool readingAll_;
};

inline void LocalFile::write(std::string_view str) {
    write(str.data(), str.size());
}
} // namespace hulatang::file

#endif // HULATANG_FILE_LOCALFILE_HPP
