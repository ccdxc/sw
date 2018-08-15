package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "p4pd_includes",
    srcs = [],
    hdrs = [
        "p4pd_api.hpp",
        "p4pd_utils.hpp",
    ],
    deps = [
    ]
)


cc_library(
    name = "p4pd",
    srcs = [
        "p4pd_utils.cc",
        "p4pd_api.cc",
        "p4pluspd_rxdma_api.cc",
        "p4pluspd_txdma_api.cc",
    ],
    hdrs = glob([
        "*.hpp",
        "*.h",
    ]),
    deps = [
        "//nic/include:base_includes",
        "//nic/hal/pd:hal_pd_includes",
        "//nic:gen_common_includes",
        "//nic:boost",
        "//nic:spdlog",
        "//nic:asic_includes",
        "//nic/hal/pd/capri:capri_includes",
        "//nic/hal/iris/datapath/p4/include:p4_iris_includes",
        "//nic/p4/common:p4_common_includes",
    ],
    alwayslink = 1,
)

cc_library(
    name = "p4pd_utils",
    srcs = [
        "p4pd_utils.cc",
    ],
    hdrs = [
        "p4pd_utils.hpp",
    ],
    deps = [
    ],
    alwayslink = 1,
)
