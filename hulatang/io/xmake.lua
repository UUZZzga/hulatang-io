local sources = {
    "CycleEventManager.cpp",
    "CycleEventWatcher.cpp",
    "EventLoop.cpp",
    "EventWatcher.cpp",
    "FdEventManager.cpp",
    "IdleEventManager.cpp",
    "TimerEventManager.cpp",
}

target("hulatang_io")
    set_kind("static")
    add_deps("hulatang_base")
    add_packages("concurrentqueue", {public = true})

    set_options("disable_logging")
    for _, src in ipairs(sources) do
        add_files(src)
    end

includes("tests")