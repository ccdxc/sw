package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "haltestutils",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/slab:slab",
        "//nic/utils/indexer:indexer",
        "//nic/utils/ht:ht",
        "//nic/utils/thread:thread",
        "//nic/hal/pd/iris:iris_includes",
    ],
)
