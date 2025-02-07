cc_binary(
    name = "echo",
    srcs = glob(["main.cc"]),
    linkopts = [
        "-lboost_context",
        "-luring",
        "-lprofiler",
    ],
    deps = [
        ":echolib",
    ],
)

cc_library(
    name = "echolib",
    hdrs = glob(["*.h"]),
    srcs = glob(["*.cc"], exclude=["main.cc"]),
    visibility = ["//visibility:public"],
    deps = [
        "//puddle",
        "//puddle/log",
        "//puddle/net",
        "//puddle/stats",
        "@abseil-cpp//absl/time",
    ],
)
