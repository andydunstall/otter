cc_binary(
    name = "echo",
    srcs = glob(["*.cc"]),
    linkopts = [
        "-lboost_context",
        "-luring",
        "-lprofiler",
    ],
    deps = [
        ":echo_lib",
        "//puddle/config",
        "//puddle/log",
        "//puddle/net",
        "//puddle/reactor",
        "@fmt",
    ],
)

cc_library(
    name = "echo_lib",
    hdrs = glob(["*.h"]),
    srcs = glob(["*.cc"], exclude=["main.cc"]),
    visibility = ["//visibility:public"],
    deps = [
        "//puddle/config",
        "//puddle/log",
        "//puddle/net",
        "//puddle/reactor",
        "@fmt",
    ],
)
