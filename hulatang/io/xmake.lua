local sources = {
    "Channel.cpp",
    "CycleEventManager.cpp",
    "CycleEventWatcher.cpp",
    "EventLoop.cpp",
    "EventWatcher.cpp",
    "FdEventManager.cpp",
    "IdleEventManager.cpp",
    "IdleEventWatcher.cpp",
    "InvokeTimer.cpp",
    "SocketChannel.cpp",
    "TCPConnection.cpp",
    "TimerEventManager.cpp",
    "TimerEventWatcher.cpp",
}

target("hulatang_io")
    set_kind("static")
    add_deps("hulatang_base")
    add_packages("concurrentqueue", {public = true})

    set_options("disable_logging")
    for _, src in ipairs(sources) do
        add_files(src)
    end
    if is_host("windows") then
        add_files("async/IOCPFdEventManager.cpp")
    end

includes("tests")