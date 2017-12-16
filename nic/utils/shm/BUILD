package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "shmseg",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
    ],
    copts = sdk_copts,
)
cc_test(
    name = "shmseg_test",
    srcs = ["test/shmseg_test.cc"],
    deps = [
        ":shmseg",
        "//nic/utils/trace",
        #"//nic:sdk_logger",
        "@sdk//obj:sdk_logger",
        # External
        "//:gtest",
    ],
    linkopts = ["-lrt"],
    copts = [
        "-Wno-write-strings",
        ] + sdk_copts,
)
