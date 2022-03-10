target("eventLoop_test")
    set_kind("binary")
    set_default(false)
    set_group("test")

    add_deps("hulatang_io")
    add_files("EventLoop_test.cpp")

target("tcpclient_test")
    set_kind("binary")
    set_default(false)
    set_group("test")

    add_deps("hulatang_io")
    add_files("TCPClient_test.cpp")

target("tcpserver_test")
    set_kind("binary")
    set_default(false)
    set_group("test")

    add_deps("hulatang_io")
    add_files("TCPServer_test.cpp")

target("eventloopthread_test")
    set_kind("binary")
    set_default(false)
    set_group("test")

    add_deps("hulatang_io")
    add_files("EventLoopThread_test.cpp")

target("eventloopthreadpool_test")
    set_kind("binary")
    set_default(false)
    set_group("test")

    add_deps("hulatang_io")
    add_files("EventLoopThreadPool_test.cpp")

target("inetaddress_test")
    set_kind("binary")
    set_default(false)
    set_group("test")

    add_deps("hulatang_io")
    add_files("InetAddress_test.cpp")
