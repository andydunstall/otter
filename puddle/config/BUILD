cc_library(
    name = "config",
    hdrs = glob(["*.h"]),
    srcs = glob(["*.cc"], exclude=["*_test.cc"]),
    visibility = ["//visibility:public"],
    deps = [
        "@yaml-cpp",
    ],
)

cc_test(
    name = "config_test",
    srcs = ["config_test.cc"],
    deps = [
        ":config",
        "@googletest//:gtest",
    ],
)
