package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

common_deps = [
    "//nic/hal:hal_hdrs",
    "//nic/utils/fsm:fsm",
    "//nic/fte:fte_includes",
    "//nic/include:base_includes",
    "//nic/hal/pd/common:pdcommon",
    "//nic/utils/ht:ht",
    "//nic/utils/slab:slab",
    "//nic/utils/indexer:indexer",
    "//nic/utils/thread:thread",
    "//nic/utils/twheel:twheel",
    "//nic/third-party/spdlog",
    "//nic:gen_proto_includes",
    "//nic:gen_includes",
    "//nic/utils/catalog:catalog",
]

cc_library(
    name = "network",
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
    name = "network_includes",
    hdrs = ["net_plugin.hpp"],
)
