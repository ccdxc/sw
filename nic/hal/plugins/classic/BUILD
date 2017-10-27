package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "classic",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic:gen_includes",
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/third-party/spdlog",
        "//nic:gen_proto_includes",
    ],
    alwayslink = 1,
)

cc_library(
    name = "classic_includes",
    hdrs = ["classic_plugin.hpp"],
)
