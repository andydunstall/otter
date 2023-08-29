cc_binary(
   name = "db",
   srcs = ["main.cc"],
   linkopts = ["-lboost_fiber", "-lboost_context", "-luring", "-lrocksdb", "-lzstd", "-lsnappy", "-lbz2", "-llz4", "-lz"],
   deps = [
     "@com_google_absl//absl/flags:flag",
     "@com_google_absl//absl/flags:parse",
     "@com_google_absl//absl/log:initialize",
     "@com_google_absl//absl/log:log",
     "//fuddle/server",
     "//puddle",
   ],
)
