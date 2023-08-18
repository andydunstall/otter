cc_library(
  name = "puddle",
  hdrs = glob(["*.h"]),
  srcs = glob(["*.cc"], exclude=["*_test.cc", "*_benchmark.cc"]),
  include_prefix = "puddle",
  visibility = ["//visibility:public"],
  deps = [
    "@com_google_absl//absl/log:check",
    "@com_google_absl//absl/log:log",
    "@com_google_absl//absl/status",
    "@com_google_absl//absl/status:statusor",
  ],
)
