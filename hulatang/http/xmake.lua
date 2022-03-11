local sources = {
    "HttpClient.cpp",
    "HttpRequest.cpp",
    "URL.cpp"
}

target("hulatang_http")
    set_kind("static")
    add_deps("hulatang_io")

    for _, src in ipairs(sources) do
        add_files(src)
    end

includes("tests")