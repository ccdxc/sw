package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "pdcommon",
    srcs = [
        "asic_pd.cc",
        "hal_pd.cc",
        "pd.hpp",
        "pd_api.hpp",
    ],
    hdrs = [
        "cpupkt_api.hpp",
        "cpupkt_headers.hpp",
        "pd.hpp",
        "pd_api.hpp",
    ],
    copts = sdk_copts,
    deps = [
        "//nic:sdk_ht",
        "//nic/hal/pd:hal_pd_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/list:list_includes",
        "//nic/utils/thread",
    ],
    alwayslink = 1,
)
