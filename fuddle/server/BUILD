cc_binary(
	name = "server",
	srcs = ["main.cc"],
	linkopts = ["-lboost_fiber", "-lboost_context", "-luring"],
	deps = [
		"@com_google_absl//absl/flags:flag",
		"@com_google_absl//absl/flags:parse",
		"@com_google_absl//absl/log:initialize",
		"@com_google_absl//absl/log:log",
		":fuddle",
		"//puddle",
	],
)

cc_library(
  name = "fuddle",
  hdrs = glob(["*.h"]),
  srcs = glob(["*.cc"], exclude=["main.cc", "*_test.cc", "*_benchmark.cc"]),
  include_prefix = "fuddle/server",
  visibility = ["//visibility:public"],
  deps = [
    "@com_google_absl//absl/log:check",
    "@com_google_absl//absl/log:log",
    "@com_google_absl//absl/status",
    "@com_google_absl//absl/status:statusor",
    "//puddle",
  ],
)
