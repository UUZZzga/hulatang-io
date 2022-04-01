local sources = {
    "Exception.cpp",
    "Log.cpp",
}

target("hulatang_base")
    set_kind("static")
    add_packages("fmt", "spdlog", {public = true})
    add_packages("backward-cpp")
    add_deps("config")

    set_options("disable_logging")
    for _, src in ipairs(sources) do
        add_files(src)
    end
    if is_host("windows") then
        add_files("platform/win32/File.cpp")
    else
        add_files("platform/posix/File.cpp")
        add_files("Socket.cpp")
    end

includes("tests")