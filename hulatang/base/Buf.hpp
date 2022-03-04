#ifndef HULATANG_BASE_BUF_HPP
#define HULATANG_BASE_BUF_HPP

namespace hulatang::base {
struct Buf
{
    Buf(char *_buf, size_t _len)
        : buf(_buf)
        , len(_len)
    {}

    char *buf;
    size_t len;
};
} // namespace hulatang::base

#endif // HULATANG_BASE_BUF_HPP
