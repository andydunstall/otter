cc_library(
    name = "log",
    hdrs = glob(["*.h"]),
    srcs = glob(["*.cc"], exclude=["*_bench.cc"]),
    visibility = ["//visibility:public"],
    deps = [
        "//puddle/shard",
        "@fmt",
    ],
)

cc_binary(
    name = "log_bench",
    srcs = ["log_bench.cc"],
    deps = [
        ":log",
        "@google_benchmark//:benchmark",
    ],
)
