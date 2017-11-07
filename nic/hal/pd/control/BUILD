package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "control",
    srcs = [
            ] + glob(["*.cc"]),
    hdrs = [
    ] + glob(["*.hpp"]) + glob(["*.h"]),
    deps = [
        "//nic/hal:hal_includes",
        "//nic:asic_includes",
        "//nic/include:base_includes",
        "//nic:gen_proto_includes",
        "//nic/utils/indexer:indexer",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/thread",
        "//nic/utils/twheel",
        "//nic/utils/catalog"
    ],
    alwayslink = 1,
)
