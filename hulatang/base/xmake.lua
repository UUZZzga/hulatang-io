local sources = {
    "Log.cpp",
}

target("hulatang_base")
    set_kind("static")
    add_packages("fmt", "spdlog", {public = true})

    set_options("disable_logging")
    for _, src in ipairs(sources) do
        add_files(src)
    end

includes("tests")