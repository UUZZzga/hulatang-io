#include "hulatang/base/Log.hpp"

#include "spdlog/sinks/stdout_color_sinks.h"
#include <memory>

namespace hulatang::base {
std::shared_ptr<spdlog::logger> Log::CoreLogger;
std::shared_ptr<spdlog::logger> Log::ClientLogger;

void Log::init()
{
    spdlog::set_pattern("%^[%T.%e]-%t-%l %n: %v%$");
    CoreLogger = spdlog::stdout_color_mt("HULATANG");
    CoreLogger->set_level(spdlog::level::info);
    ClientLogger = spdlog::stdout_color_mt("APP");
    ClientLogger->set_level(spdlog::level::info);
}

} // namespace hulatang::base