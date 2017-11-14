package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "isc-dhcp",
    srcs = [
        "common/alloc.c",
        "common/options.c",
        "common/parse.c",
        "common/tables.c",
        "common/tree.c",
        "dhcp_pen_api.c",
        "omapip/alloc.c",
        "omapip/errwarn.c",
        "omapip/hash.c",
    ],
    hdrs = [
        "includes/dhcp.h",
        "includes/dhcp6.h",
        "includes/dhcp_internal.h",
        "includes/omapip/alloc.h",
        "includes/omapip/hash.h",
        "includes/tree.h",
    ],
    copts = [
        "-x",
        "c++",
        "--std=c++11",
        "-Wno-misleading-indentation",
    ],
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
    ],
    alwayslink = 1,
)
