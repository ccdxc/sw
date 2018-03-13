package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "alg_utils",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/fte:fte",
        "//nic/hal:hal_src",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
    ],
    alwayslink = 1,
    copts = sdk_copts,
)

cc_library(
    name = "alg_utils_includes",
    hdrs = ["core.hpp",
            "alg_db.hpp",
            "mime.hpp",
    ],
)    
