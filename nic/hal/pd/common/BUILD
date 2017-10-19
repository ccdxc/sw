package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pdcommon",
    srcs = [
        "hal_pd.cc",
        "asic_pd.cc",
        "pd.hpp",
        "pd_api.hpp",
    ],
    hdrs = [
        "cpupkt_api.hpp",
        "pd.hpp",
        "pd_api.hpp",
        "cpupkt_headers.hpp",
    ],
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/ht:ht",
        "//nic/utils/list:list_includes",
        "//nic/utils/thread:thread",
    ],
    alwayslink = 1,
)
