#include "hulatang/base/Log.hpp"

#include "spdlog/async.h"
#include "spdlog/common.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <memory>

namespace hulatang::base {
std::shared_ptr<spdlog::logger> Log::CoreLogger;
std::shared_ptr<spdlog::logger> Log::ClientLogger;

void Log::init()
{
    spdlog::set_pattern("%^[%T.%e]-%t-%l %n: %v%$");
    CoreLogger = spdlog::stdout_color_mt("HULATANG");
    // CoreLogger = spdlog::create_async<spdlog::sinks::basic_file_sink_mt>("HULATANG", "logs/async_log.txt");
    CoreLogger->set_level(spdlog::level::info);
    ClientLogger = spdlog::stdout_color_mt("APP");
    // ClientLogger = spdlog::create_async<spdlog::sinks::basic_file_sink_mt>("APP", "logs/async_log.txt");
    ClientLogger->set_level(spdlog::level::info);
}

} // namespace hulatang::base