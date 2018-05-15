package(default_visibility = ["//visibility:public"])

# compile the service and reactor
cc_library(
    name = "upgrade_lib",
    srcs = [
        "upgrade.cc",
        "upgrade.hpp",
    ],
    hdrs = [
        "upgrade.hpp",
    ],
    deps = [
        "//nic/delphi/sdk",
        "//nic/upgrade_manager/proto:upgrade_gen",
    ],
)
