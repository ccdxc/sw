package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pal",

    srcs = [
        "pal.cc",
        "pal_sim.cc",
        "pal_hw.cc",
    ],

    hdrs = [
    ] + glob(["*.hpp"]),

    deps = [
        "//nic/utils/list:list_includes",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/third-party/spdlog",
    ],

    copts = [
        "-DCAPRI_HAL"
    ],

    alwayslink = 1,
)
