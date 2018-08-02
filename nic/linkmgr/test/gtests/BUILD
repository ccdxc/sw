
package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_test(
    name = "port_test",
    srcs = ["port_test.cc"],
    deps = [
        "//nic/linkmgr/test/utils:linkmgr_gtest_utils",
        # External
        "//:gtest",
    ],
    linkopts = [
        "-ldl",
    ],
    copts = sdk_copts,
)
