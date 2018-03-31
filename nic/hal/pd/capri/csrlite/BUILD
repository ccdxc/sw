package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_binary(
    name = "libcsrlite.so",
    linkshared = 1,
    srcs = glob(["*.cc"]),
    deps = [
       "@sdk//:csrlite_gen_includes",
       "//nic:csrlite_gen",
    ],
)
