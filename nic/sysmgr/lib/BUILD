package(default_visibility = ["//visibility:public"])

cc_library(
    name = "includes",
    srcs = [],
    hdrs = ["sysmgr_client.hpp"],
)

cc_library(
    name = "lib",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/delphi/sdk",
        "//nic/sysmgr/proto:sysmgr_gen",
        "//nic:spdlog",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
)
