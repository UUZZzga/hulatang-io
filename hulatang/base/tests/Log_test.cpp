#include "hulatang/base/Log.hpp"

int main(int _argc, const char **_argv) {
    hulatang::base::Log::init();
    HLT_CORE_INFO("Hello");
    HLT_WARN("Hello");
    return 0;
}