package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "alg",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic:gen_includes",
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/third-party/spdlog",
        "//nic:libtirpc",
    ],
    alwayslink = 1,
)

cc_library(
    name = "alg_includes",
    hdrs = ["alg_tftp.hpp", "alg_rpc.hpp"],
)
