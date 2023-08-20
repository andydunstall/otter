cc_binary(
	name = "echo",
	srcs = ["main.cc"],
	linkopts = ["-lboost_fiber", "-lboost_context", "-luring"],
	deps = [
		"@com_google_absl//absl/flags:flag",
		"@com_google_absl//absl/flags:parse",
		"@com_google_absl//absl/log:initialize",
		"@com_google_absl//absl/log:log",
		"//puddle",
	],
)
