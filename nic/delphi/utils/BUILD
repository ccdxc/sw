package(default_visibility = ["//visibility:public"])

cc_library(
    name = "utils",
    srcs = [
    ] + glob(["*.cc"], exclude = ["*_test.cc"],),
    hdrs = [
    ] + glob(["*.hpp"]),
    deps = [
        "//nic:spdlog",
    ],
    include_prefix = "nic/delphi/utils",
)

cc_test(
    name = "utils_test",
    srcs = [
    ] + glob(["*_test.cc"]),
    deps = [
        ":utils",
        "//:gtest",
    ],
    linkopts = [
        "-L/usr/local/lib",
    ],
    timeout="short",
)
