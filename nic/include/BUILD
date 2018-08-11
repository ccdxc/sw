package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

# TODO: this should go away, it has all kinds of pipeline
#       specific stuff, most files from this dir should
#       be moved to their respective dirs
cc_library(
    name = "base_includes",
    srcs = [],
    hdrs = glob(["*.hpp", "*.h"]),
    deps = [
        "@sdk//:sdk_headers",
        "//nic:boost",
        "//nic:grpc_includes",
    ]
)

cc_library(
    name = "common_includes",
    srcs = [],
    hdrs = [
        "base.hpp",
        "trace.hpp",
        "hal_cfg.hpp",
    ],
    deps = [
        "@sdk//:sdk_headers",
        "//nic:boost",
        "//nic:grpc_includes",
    ]
)
