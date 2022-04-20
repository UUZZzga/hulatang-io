target("example_pingpong_client")
    set_kind("binary")
    set_default(false)
    set_group("example")

    add_deps("hulatang_io")
    add_files("client.cpp")

target("example_pingpong_server")
    set_kind("binary")
    set_default(false)
    set_group("example")

    add_deps("hulatang_io")
    add_files("server.cpp")
