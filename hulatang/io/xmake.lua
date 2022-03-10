local sources = {
    "Channel.cpp",
    "Connector.cpp",
    "CycleEventManager.cpp",
    "CycleEventWatcher.cpp",
    "EventLoop.cpp",
    "EventLoopThread.cpp",
    "EventLoopThreadPool.cpp",
    "EventWatcher.cpp",
    "FdEventManager.cpp",
    "IdleEventManager.cpp",
    "IdleEventWatcher.cpp",
    "InetAddress.cpp",
    "InvokeTimer.cpp",
    "SocketChannel.cpp",
    "TCPClient.cpp",
    "TCPConnection.cpp",
    "TCPServer.cpp",
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
        local win32_sources = {
            "async/IOCPFdEventManager.cpp",
            "async/Win32Acceptor.cpp"
        }
        for _, src in ipairs(win32_sources) do
            add_files(src)
        end
    end

includes("tests")