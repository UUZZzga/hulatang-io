#ifndef HULATANG_BASE_BUFFER_HPP
#define HULATANG_BASE_BUFFER_HPP

#include "hulatang/base/NoInit.hpp"

#include <cassert>
#include <bit>
#include <algorithm>
#include <cstring>
#include <string_view>
#include <vector>

namespace hulatang::base {
template<std::endian E, typename T>
constexpr T bit_endian(T i)
{
    if constexpr (std::endian::native != E)
    {
        if constexpr (sizeof(T) == 8)
        {
            return ((i & 0x00000000000000FF) << 56 | (i & 0x000000000000FF00) << 40 | (i & 0x0000000000FF0000) << 24 |
                    (i & 0x00000000FF000000) << 8 | (i & 0x000000FF00000000) >> 8 | (i & 0x0000FF0000000000) >> 24 |
                    (i & 0x00FF000000000000) >> 40 | (i & 0xFF00000000000000) >> 56);
        }
        if constexpr (sizeof(T) == 4)
        {
            return ((i & 0x000000FF) << 24 | (i & 0x0000FF00) << 8 | (i & 0x00FF0000) >> 8 | (i & 0xFF000000) >> 24);
        }
        if constexpr (sizeof(T) == 2)
        {
            return ((i & 0x00FF) << 8 | (i & 0xFF00) >> 8);
        }
    }
    return i;
}

// Modified from muduo
template<std::endian endian>
class BufferEndian
{
public:
    static constexpr size_t kCheapPrepend = 8;
    static constexpr size_t kInitialSize = 1024;

    explicit BufferEndian(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == initialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    BufferEndian(const BufferEndian &other) = default;

    BufferEndian(BufferEndian &&other) noexcept
        : readerIndex_(0)
        , writerIndex_(0)
    {
        swap(other);
    }

    ~BufferEndian() = default;
    BufferEndian &operator=(const BufferEndian &other) = default;

    BufferEndian &operator=(BufferEndian &&other) noexcept
    {
        swap(other);
        BufferEndian().swap(other);
        return *this;
    }

    void swap(BufferEndian &rhs) noexcept
    {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }

    [[nodiscard]] size_t readableBytes() const
    {
        return writerIndex_ - readerIndex_;
    }

    [[nodiscard]] size_t size() const
    {
        return readableBytes();
    }

    [[nodiscard]] size_t writableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }

    [[nodiscard]] size_t prependableBytes() const
    {
        return readerIndex_;
    }

    [[nodiscard]] const char *peek() const noexcept
    {
        return begin() + readerIndex_;
    }

    [[nodiscard]] char *peek() noexcept
    {
        return begin() + readerIndex_;
    }

    [[nodiscard]] const char *data() const
    {
        return peek();
    }

    [[nodiscard]] char *data()
    {
        return peek();
    }

    [[nodiscard]] const char *findCRLF() const
    {
        const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    [[nodiscard]] const char *findCRLF(const char *start) const
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const char *crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    [[nodiscard]] const char *findEOL() const
    {
        const void *eol = memchr(peek(), '\n', readableBytes());
        return static_cast<const char *>(eol);
    }

    [[nodiscard]] const char *findEOL(const char *start) const
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const void *eol = memchr(start, '\n', beginWrite() - start);
        return static_cast<const char *>(eol);
    }

    // retrieve returns void, to prevent
    // string str(retrieve(readableBytes()), readableBytes());
    // the evaluation of two functions are unspecified
    void retrieve(size_t len)
    {
        assert(len <= readableBytes());
        if (len < readableBytes())
        {
            readerIndex_ += len;
        }
        else
        {
            retrieveAll();
        }
    }

    void retrieveUntil(const char *end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    void retrieveInt64()
    {
        retrieve(sizeof(int64_t));
    }

    void retrieveInt32()
    {
        retrieve(sizeof(int32_t));
    }

    void retrieveInt16()
    {
        retrieve(sizeof(int16_t));
    }

    void retrieveInt8()
    {
        retrieve(sizeof(int8_t));
    }

    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    [[nodiscard]] std::string_view toString_view() const
    {
        return {peek(), readableBytes()};
    }

    void append(const BufferEndian &buffer)
    {
        append(buffer.begin(), buffer.readableBytes());
    }
    void append(std::string_view str)
    {
        append(str.data(), str.size());
    }

    void append(const char * /*restrict*/ data, size_t len)
    {
        ensureWritableBytes(len);
        // std::copy(data, data + len, beginWrite());
        memcpy(beginWrite(), data, len);
        hasWritten(len);
    }

    void append(const void * /*restrict*/ data, size_t len)
    {
        append(static_cast<const char *>(data), len);
    }

    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    char *beginWrite() noexcept
    {
        return begin() + writerIndex_;
    }

    [[nodiscard]] const char *beginWrite() const
    {
        return begin() + writerIndex_;
    }

    void hasWritten(size_t len)
    {
        assert(len <= writableBytes());
        writerIndex_ += len;
    }

    void unwrite(size_t len)
    {
        assert(len <= readableBytes());
        writerIndex_ -= len;
    }

    ///
    /// Append int64_t using network endian
    ///
    void appendInt64(int64_t x)
    {
        int64_t be64 = bit_endian<endian>(x);
        append(&be64, sizeof be64);
    }

    ///
    /// Append int32_t using network endian
    ///
    void appendInt32(int32_t x)
    {
        int32_t be32 = bit_endian<endian>(x);
        append(&be32, sizeof be32);
    }

    void appendInt16(int16_t x)
    {
        int16_t be16 = bit_endian<endian>(x);
        append(&be16, sizeof be16);
    }

    void appendInt8(int8_t x)
    {
        append(&x, sizeof x);
    }

    ///
    /// Read int64_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int32_t)
    int64_t readInt64()
    {
        int64_t result = peekInt64();
        retrieveInt64();
        return result;
    }

    ///
    /// Read int32_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int32_t)
    int32_t readInt32()
    {
        int32_t result = peekInt32();
        retrieveInt32();
        return result;
    }

    int16_t readInt16()
    {
        int16_t result = peekInt16();
        retrieveInt16();
        return result;
    }

    int8_t readInt8()
    {
        int8_t result = peekInt8();
        retrieveInt8();
        return result;
    }

    ///
    /// Peek int64_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int64_t)
    [[nodiscard]] int64_t peekInt64() const
    {
        assert(readableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        std::copy(&be64, peek(), sizeof be64);
        return bit_endian<endian>(be64);
    }

    ///
    /// Peek int32_t from network endian
    ///
    /// Require: buf->readableBytes() >= sizeof(int32_t)
    [[nodiscard]] int32_t peekInt32() const
    {
        assert(readableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        std::copy(&be32, peek(), sizeof be32);
        return bit_endian<endian>(be32);
    }

    [[nodiscard]] int16_t peekInt16() const
    {
        assert(readableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        std::copy(&be16, peek(), sizeof be16);
        return bit_endian<endian>(be16);
    }

    [[nodiscard]] int8_t peekInt8() const
    {
        assert(readableBytes() >= sizeof(int8_t));
        int8_t x = *peek();
        return x;
    }

    ///
    /// Prepend int64_t using network endian
    ///
    void prependInt64(int64_t x)
    {
        int64_t be64 = bit_endian<endian>(x);
        prepend(&be64, sizeof be64);
    }

    ///
    /// Prepend int32_t using network endian
    ///
    void prependInt32(int32_t x)
    {
        int32_t be32 = bit_endian<endian>(x);
        prepend(&be32, sizeof be32);
    }

    void prependInt16(int16_t x)
    {
        int16_t be16 = bit_endian<endian>(x);
        prepend(&be16, sizeof be16);
    }

    void prependInt8(int8_t x)
    {
        prepend(&x, sizeof x);
    }

    void prepend(const void * /*restrict*/ data, size_t len)
    {
        assert(len <= prependableBytes());
        readerIndex_ -= len;
        const char *d = static_cast<const char *>(data);
        std::copy(d, d + len, begin() + readerIndex_);
    }

    void shrink(size_t reserve)
    {
        // FIXME: use vector::shrink_to_fit() in C++ 11 if possible.
        BufferEndian other;
        other.ensureWritableBytes(readableBytes() + reserve);
        other.append(toString_view());
        swap(other);
    }

    [[nodiscard]] size_t internalCapacity() const
    {
        return buffer_.capacity();
    }

private:
    [[nodiscard]] char *begin() noexcept
    {
        return &*buffer_.begin();
    }

    [[nodiscard]] const char *begin() const noexcept
    {
        return &*buffer_.begin();
    }

    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            // move readable data to the front, make space inside buffer
            assert(kCheapPrepend < readerIndex_);
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
            assert(readable == readableBytes());
        }
    }

private:
    std::vector<NoInit<char>> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    static constexpr char kCRLF[] = "\r\n";
};

using Buffer = BufferEndian<std::endian::native>;
} // namespace hulatang::base

#endif // HULATANG_BASE_BUFFER_HPP
