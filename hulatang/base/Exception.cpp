#include "hulatang/base/Exception.hpp"

#include <backward/backward.hpp>
#include <sstream>

namespace {
inline std::string createStackTrace()
{
    namespace ColorMode = backward::ColorMode;
    using backward::Printer;
    using backward::StackTrace;
    StackTrace st;
    st.load_here(32);
    Printer p;
    p.object = true;
    p.color_mode = ColorMode::always;
    p.address = true;

    std::stringstream stack;
    p.print(st, stack);
    return stack.str();
}
} // namespace

namespace hulatang::base {
Exception::Exception(std::string what)
    : message(std::move(what))
    , stack(createStackTrace())
{}
} // namespace hulatang::base