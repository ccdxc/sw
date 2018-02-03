package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

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
        "//nic:gen_includes",
        "//nic:boost",
        "//nic:spdlog",
        "//nic:asic_includes",
        "//nic/hal/pd/capri:capri_includes",
    ],
    alwayslink = 1,
)
