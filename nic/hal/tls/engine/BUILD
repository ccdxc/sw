package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "pse",
    srcs = glob(["*.c"]) + glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    deps = [
        "//nic:openssl",
        "//nic/utils/trace",
        "//nic/hal/pd:hal_pd_includes",
    ],
)
