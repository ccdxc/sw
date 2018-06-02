package(default_visibility = ["//visibility:public"])

# compile the example service library
cc_library(
    name = "example_lib",
    srcs = [
        "example.cc",
        "example.hpp",
    ],
    hdrs = [
        "example.hpp",
    ],
    deps = [
        "//nic/delphi/sdk",
        "//nic/upgrade_manager/export/c:upgrade_app_lib",
        "//nic/upgrade_manager/export/c:upgrade_app_lib_includes",
    ],
)

# Compile the example binary
cc_binary(
    name = "example",
    srcs = [
        "example_main.cc"
    ],
    deps = [
        ":example_lib",
        "//nic/upgrade_manager/export/c:upgrade_app_lib",
        "//nic/upgrade_manager/export/c:upgrade_app_lib_includes",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    linkopts = [
        "-lpthread",
    ],
)

cc_library(
    name = "nmd_includes",
    srcs = [],
    hdrs = ["nmd.hpp"],
)

#nmd
cc_binary(
    name = "nmd",
    srcs = [
        "nmd.cc"
    ],
    deps = [
        "//nic/upgrade_manager/export/c:upgrade_app_lib",
        "//nic/upgrade_manager/export/c:upgrade_app_lib_includes",
        ":nmd_includes",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    linkopts = [
        "-lpthread",
    ],
)
