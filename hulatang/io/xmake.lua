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

local win32_sources = {
    "async/IOCPFdEventManager.cpp",
    "async/Win32Acceptor.cpp"
}

local linux_sources = {
    "accept/PosixAcceptor.cpp",
    "async/EPollFdEventManager.cpp",
    "async/LinuxFdEventManager.cpp",
}

target("hulatang_io")
    set_kind("static")
    add_deps("hulatang_base")
    add_packages("concurrentqueue", {public = true})

    set_options("disable_logging")
    for _, src in ipairs(sources) do
        add_files(src)
    end
    local platform_sources
    if is_host("windows") then
        platform_sources = win32_sources
    elseif  is_host("linux") then
        platform_sources = linux_sources
    end
    for _, src in ipairs(platform_sources) do
        add_files(src)
    end

includes("extend")
includes("tests")