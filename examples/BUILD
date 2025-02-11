cc_binary(
    name = "echo",
    srcs = glob(["echo.cc"]),
    linkopts = [
        "-lboost_context",
        "-luring",
        "-lprofiler",
    ],
    deps = [
        "//puddle",
        "//puddle/cli",
        "//puddle/log",
        "//puddle/net",
    ],
)

cc_binary(
    name = "tasks",
    srcs = glob(["tasks.cc"]),
    linkopts = [
        "-lboost_context",
        "-luring",
        "-lprofiler",
    ],
    deps = [
        "//puddle",
    ],
)
