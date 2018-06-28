package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "crc_fast",
    srcs = ["crc_fast.cc", "crc_fast.hpp"],
    hdrs = ["crc_fast.hpp"],
    deps = ["//nic/include:base_includes", "//nic:spdlog", "//nic/hal:hal_includes"],
    copts = sdk_copts,
)
