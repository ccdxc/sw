package(default_visibility = ["//visibility:public"])

cc_library(
    name = "upgrade_app_lib_includes",
    srcs = [],
    hdrs = ["upgrade.hpp"],
)

# compile the reactor for apps
cc_library(
    name = "upgrade_app_lib",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/delphi/sdk",
        "//nic/upgrade_manager/proto:upgrade_gen",
    ],
)
