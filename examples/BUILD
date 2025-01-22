cc_binary(
    name = "echo",
    srcs = glob(["echo.cc"]),
    linkopts = [
        "-lboost_context",
        "-luring",
        "-lprofiler",
    ],
    deps = [
        "//puddle/config",
        "//puddle/log",
        "//puddle/pprof",
        "//puddle/net",
        "//puddle/reactor",
        "//puddle/stats",
        "@abseil-cpp//absl/time",
        "@fmt",
    ],
)
