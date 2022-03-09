local sources = {
    "LocalFile.cpp",
}

target("hulatang_file")
    set_kind("static")
    add_deps("hulatang_io")

    set_options("disable_logging")
    for _, src in ipairs(sources) do
        add_files(src)
    end

includes("tests")