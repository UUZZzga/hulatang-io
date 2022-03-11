#ifndef HULATANG_BASE_STREAMBUFFER_HPP
#define HULATANG_BASE_STREAMBUFFER_HPP

#include "hulatang/base/Buffer.hpp"
#include <ios>
#include <streambuf>

namespace hulatang::base {
template<std::endian ENDIAN>
class StreamBufferEndian : public std::streambuf
{
public:
    using streamsize = std::streamsize;

    explicit StreamBufferEndian(BufferEndian<ENDIAN> *buf)
        : buf_(buf)
    {}

    streamsize xsputn(const char *s, streamsize count) override
    {
        if (count > 0)
        {
            buf_->append(s, count);
            return count;
        }
        return 0;
    }

    int_type overflow(int_type ch) override
    {
        buf_->appendInt8(ch);
        return ch;
    }

    int sync() override
    {
        return 0;
    }

private:
    BufferEndian<ENDIAN> *buf_;
};
} // namespace hulatang::base

#endif // HULATANG_BASE_STREAMBUFFER_HPP
