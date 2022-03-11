target("url_test")
    set_kind("binary")
    set_default(false)
    set_group("test")

    add_deps("hulatang_http")
    add_files("URL_test.cpp")