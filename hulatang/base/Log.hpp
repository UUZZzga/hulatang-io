#ifndef HULATANG_BASE_LOG_HPP
#define HULATANG_BASE_LOG_HPP

#include <spdlog/spdlog.h>

namespace hulatang::base {
class Log
{
public:
    static void init();

    static std::shared_ptr<spdlog::logger> &getCoreLogger()
    {
        return CoreLogger;
    }
    static std::shared_ptr<spdlog::logger> &getClientLogger()
    {
        return ClientLogger;
    }
private:
    static std::shared_ptr<spdlog::logger> CoreLogger;
    static std::shared_ptr<spdlog::logger> ClientLogger;
};

} // namespace hulatang::base

#ifndef DISABLE_LOGGING
// core log macros
#    define HLT_CORE_TRACE(...) ::hulatang::base::Log::getCoreLogger()->trace(__VA_ARGS__)
#    define HLT_CORE_DEBUG(...) ::hulatang::base::Log::getCoreLogger()->debug(__VA_ARGS__)
#    define HLT_CORE_INFO(...) ::hulatang::base::Log::getCoreLogger()->info(__VA_ARGS__)
#    define HLT_CORE_WARN(...) ::hulatang::base::Log::getCoreLogger()->warn(__VA_ARGS__)
#    define HLT_CORE_ERROR(...) ::hulatang::base::Log::getCoreLogger()->error(__VA_ARGS__)
#    define HLT_CORE_FATAL(...) ::hulatang::base::Log::getCoreLogger()->critical(__VA_ARGS__)

// client log macros
#    define HLT_TRACE(...) ::hulatang::base::Log::getClientLogger()->trace(__VA_ARGS__)
#    define HLT_DEBUG(...) ::hulatang::base::Log::getClientLogger()->debug(__VA_ARGS__)
#    define HLT_INFO(...) ::hulatang::base::Log::getClientLogger()->info(__VA_ARGS__)
#    define HLT_WARN(...) ::hulatang::base::Log::getClientLogger()->warn(__VA_ARGS__)
#    define HLT_ERROR(...) ::hulatang::base::Log::getClientLogger()->error(__VA_ARGS__)
#    define HLT_FATAL(...) ::hulatang::base::Log::getClientLogger()->critical(__VA_ARGS__)
#else
// core log macros
#    define HLT_CORE_TRACE(...)
#    define HLT_CORE_DEBUG(...)
#    define HLT_CORE_INFO(...)
#    define HLT_CORE_WARN(...)
#    define HLT_CORE_ERROR(...)
#    define HLT_CORE_FATAL(...)

// client log macros
#    define HLT_TRACE(...)
#    define HLT_DEBUG(...)
#    define HLT_INFO(...)
#    define HLT_WARN(...)
#    define HLT_ERROR(...)
#    define HLT_FATAL(...)
#endif // !DISABLE_LOGGING

#endif // HULATANG_BASE_LOG_HPP
