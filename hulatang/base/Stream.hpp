#ifndef HULATANG_BASE_OUTPUTSTREAM_HPP
#define HULATANG_BASE_OUTPUTSTREAM_HPP

#include "hulatang/base/Buffer.hpp"
#include "hulatang/base/StreamBuffer.hpp"

#include <istream>
#include <ostream>

namespace hulatang::base {
template<std::endian ENDIAN>
class OutputStreamEndian : public std::ostream
{
public:
    explicit OutputStreamEndian(BufferEndian<ENDIAN> *buf)
        : std::ostream(nullptr)
        , buf_(buf)
        , streambuf_(buf)
    {
        rdbuf(&streambuf_);
    }

private:
    BufferEndian<ENDIAN> *buf_;
    StreamBufferEndian<ENDIAN> streambuf_;
};

template<std::endian ENDIAN>
class InputStreamEndian : public std::istream
{
public:
    explicit InputStreamEndian(BufferEndian<ENDIAN> *buf)
        : buf_(buf)
        , streambuf_(buf)
    {
        rdbuf(&streambuf_);
    }

private:
    BufferEndian<ENDIAN> *buf_;
    StreamBufferEndian<ENDIAN> streambuf_;
};

using InputStream = InputStreamEndian<std::endian::native>;
using OutputStream = OutputStreamEndian<std::endian::native>;
} // namespace hulatang::base

#endif // HULATANG_BASE_OUTPUTSTREAM_HPP
