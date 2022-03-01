target("log_test")
    set_kind("binary")
    set_default(false)
    set_group("test")

    add_deps("hulatang_base")
    add_files("Log_test.cpp")
