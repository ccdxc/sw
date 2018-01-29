package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "hal_pd_includes",
    srcs = [],
    hdrs = ["p4pd_api.hpp", "hal_pd_error.hpp", "asic_pd.hpp", "hal_pd.hpp"],
    deps = [
        "//nic/hal:hal_includes",
    ]
)

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
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_pal",
        "@sdk//obj:sdk_list",
        "@sdk//obj:sdk_thread",
        "//nic/hal/pd:hal_pd_includes",
        "//nic/hal/lib:hal_handle_includes",
        "//nic/include:base_includes",
        "//nic:spdlog",
    ],
    alwayslink = 1,
)
