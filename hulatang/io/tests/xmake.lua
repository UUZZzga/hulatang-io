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
