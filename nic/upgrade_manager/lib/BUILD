package(default_visibility = ["//visibility:public"])

# compile the service and reactor
cc_library(
    name = "upgrade_lib",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/delphi/sdk",
        "//nic/upgrade_manager/proto:upgrade_gen",
        "//nic/upgrade_manager/include/c:upgrade_common_includes",
    ],
)
