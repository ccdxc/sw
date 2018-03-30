package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "csr_lite_base_includes",
    hdrs = glob(["include/*.hpp"]),
    includes = [
        "include"
    ],
)

cc_binary(
    name = "libcsr_lite.so",
    linkshared = 1,
    srcs = glob(["*.cc"]),
    deps = [
        "//nic:csr_lite_gen",
    ],
)

