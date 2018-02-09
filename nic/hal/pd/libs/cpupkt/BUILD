package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "cpupkt",
    srcs = [] + glob(["*.cc"]),
    hdrs = [] + glob(["*.hpp"]) + glob(["*.h"]),
    copts = sdk_copts,
    deps = [
        "//nic:spdlog",
        "//nic/hal/pd/capri:capri_includes",
        "//nic/hal/pd/iris:iris_includes",
        "//nic:gen_proto_includes",
        "//nic/hal/pd/libs/wring:wringpd_includes",
        "//nic/asm/cpu-p4plus/include:cpu_p4plus_includes",
        "//nic/hal/pd:pdcommon",
    ],
    alwayslink = 1,
)

