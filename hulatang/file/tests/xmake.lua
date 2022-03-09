target("localfile_test")
    set_kind("binary")
    set_default(false)
    set_group("test")

    add_deps("hulatang_file")
    add_files("LocalFile_test.cpp")
