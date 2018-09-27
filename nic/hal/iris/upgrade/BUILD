# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
#
package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "upg_includes",
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/delphi/sdk",
        "//nic/upgrade_manager/export/upgcsdk:upgrade_app_lib_includes",
        "//nic/upgrade_manager/utils:utils",
    ]
)

cc_library(
    name = "upg",
    srcs = glob(["*.cc"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/hal/iris/upgrade:upg_includes",
        "//nic/delphi/sdk",
        "//nic/upgrade_manager/export/upgcsdk:upgrade_app_lib",
        "//nic/upgrade_manager/export/upgcsdk:upgrade_app_lib_includes",
        "//nic/upgrade_manager/utils:utils",
    ],
    alwayslink = 1
)

