#ifndef HULATANG_IO_MODEL_HPP
#define HULATANG_IO_MODEL_HPP

namespace hulatang::io {
class Channel;
class Model
{
public:
    virtual ~Model() = default;
    virtual void handleEvent(Channel *channel) = 0;
};
} // namespace hulatang::io

#endif // HULATANG_IO_MODEL_HPP
