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
