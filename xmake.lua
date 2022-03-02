add_rules("mode.debug", "mode.release")

set_languages("c++20")

-- 关闭打印所有日志
option("disable_logging")
    set_default(false)
    set_showmenu(true)
    add_defines("DISABLE_LOGGING")
option_end()

-- 拉取远程依赖
add_requires("gtest 1.11.0")
add_requires("spdlog 1.9.2", {system = false, configs = {fmt_external = true}})
add_requires("backward-cpp v1.6") -- 堆栈跟踪

set_warnings("all")

-- 关闭警告
if is_plat("windows") then
    add_cxxflags("/wd4819")
end

-- 配置模式行为
if is_mode("debug") then
    -- 添加DEBUG编译宏
    add_defines("_DEBUG")

    -- 启用调试符号
    set_symbols("debug")

    -- 禁用优化
    set_optimize("none")

    if is_plat("windows") then
        set_runtimes("MTd")
    end
else
    -- 添加NDEBUG编译宏
    add_defines("NDEBUG")

    -- set_optimize("fastest")    -- O3
    set_optimize("faster")    -- O2

    if is_plat("windows") then
        set_runtimes("MT")
    end
end

-- 启用协程支持
if is_plat("windows") then
    add_cxxflags("/await:strict") -- _COROUTINE_ABI = 2
elseif is_host("macosx") then
    add_cxxflags("-fcoroutines-ts")
else
    add_cxxflags("-fcoroutines","-fconcepts-diagnostics-depth=8")
end

--系统库
if is_host("windows") then
    add_syslinks("Ws2_32", "Shell32", "PSAPI", "DbgHelp")
elseif is_host("linux") then
    add_syslinks("pthread")
end

add_includedirs(".")

if is_plat("mingw") then
    -- add_cflags("-D__MSVCRT_VERSION__=0x1100","-D__USE_MINGW_ACCESS")
    -- add_cxxflags("-D__MSVCRT_VERSION__=0x1100","-D__USE_MINGW_ACCESS")
    add_ldflags("-nostdlib")
    add_syslinks("mingw32","gcc","mingwex","msvcr110")
end

includes("check_cfuncs.lua") -- configvar_check_cfuncs
includes("check_cxxsnippets.lua") -- configvar_check_cxxsnippets

target("config")
    set_kind("headeronly")
    -- 生成配置文件
    configvar_check_cfuncs("HAVE_FWRITE_UNLOCKED", 'fwrite_unlocked',{includes = {"stdio.h"}})
    configvar_check_cfuncs("HAVE__FWRITE_NOLOCK", '_fwrite_nolock',{includes = {"stdio.h"}})
    configvar_check_cfuncs("HAVE_EPOLL_CREATE1", 'epoll_create1',{includes = {"sys/epoll.h"}})
    configvar_check_cfuncs("HAVE_EPOLL_CREATE", 'epoll_create',{includes = {"sys/epoll.h"}})

    set_configdir("$(buildir)/config")
    add_includedirs("$(buildir)/config", {public = true})

    add_configfiles("hulatang/base/Config.h.in")

includes("hulatang/base")
includes("hulatang/io")