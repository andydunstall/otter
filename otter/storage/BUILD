cc_library(
  name = "storage",
  hdrs = glob(["*.h"]),
  srcs = glob(["*.cc"], exclude=["*_test.cc", "*_benchmark.cc"]),
  include_prefix = "otter/storage",
  visibility = ["//visibility:public"],
  deps = [
    "@com_google_absl//absl/log:check",
    "@com_google_absl//absl/status",
    "@com_google_absl//absl/status:statusor",
  ],
)
