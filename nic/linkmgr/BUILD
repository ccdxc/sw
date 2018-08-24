# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "csr_copts")
load("//nic/bazel:common_deps.bzl", "csr_defines_copts")

# headers for using linkmgr as lib
cc_library(
    name = "linkmgr_includes",
    hdrs = [
        "linkmgr.hpp",
    ]
)

# headers for linkmgr internal use
cc_library(
    name = "linkmgr_hdrs",
    hdrs = [
    ] + glob(["*.h", "*.hpp"])
)

# linkmgr sources
cc_library(
    name = "linkmgr_libsrc",
    srcs = [
        "linkmgr_src.cc",
        "linkmgr_svc.cc",
        "linkmgr_debug_svc.cc",
        "linkmgr_state.cc",
        "linkmgr_utils.cc",
    ],
    deps = [
        "//nic:gen_proto_includes",
        "@sdk//:csr_includes",
        "@sdk//obj:sdk_logger",
        "@sdk//obj:sdk_pal",
        "@sdk//obj:sdk_periodic",
        "@sdk//obj:sdk_shmmgr",
        "@sdk//obj:sdk_catalog",
        "@sdk//obj:sdk_utils",
        "@sdk//obj:sdk_list",
        "//nic:spdlog",
        "//nic/utils/block_list:block_list",
        "//nic/utils/mtrack:mtrack",
        "//nic/linkmgr:linkmgr_hdrs",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/hal/lib:hal_lib",
        "//nic/hal:hal_includes",
        "//nic:halproto",
        "//nic:grpc_includes",
        "//nic:libprotobuf",
        "//nic/utils/trace:trace",
        "//nic:google_includes",
        "//nic:libaapl",
        "@sdk//obj:sdk_linkmgr",
        "@sdk//obj:sdk_twheel",
    ],
    copts = sdk_copts,
    alwayslink = 1,
)

# CSR helpers
cc_library(
    name = "linkmgrcsr_helpers",
    srcs = [
        "cap_top_csr.cc",
        "cpu_hal_if.cc",
    ],
    hdrs = [
        "cap_top_csr.h",
        "cpu_hal_if.h",
    ],
    deps = [
        "@sdk//obj:sdk_pal",
        "//nic:linkmgr_capricsr_int",
    ],

    # csr related copts due to deps on csr
    copts = [
        "-Os",
        "-fvisibility=hidden",
    ] + csr_copts + csr_defines_copts + sdk_copts,
)

# Custom CSR APIs
cc_library(
    name = "linkmgrcsr",
    srcs = [
        "pknobs.cc",
        "cap_mx_api.cc",
        "pknobs.hpp",
    ],
    deps = [
        "@sdk//:csr_includes",
        "//nic/linkmgr:linkmgrcsr_helpers",
        "//nic:libsknobs",
    ],

    # csr related copts because of cap_mx_api.cc
    copts = csr_copts + csr_defines_copts,

    alwayslink = 1,
)

cc_library(
    name = "linkmgr_src",
    srcs = [
        "linkmgr.cc",
        "utils.cc",
        "linkmgr_svc.hpp",
        "linkmgr_src.hpp",
        "linkmgr_state.hpp",
        "linkmgr_debug_svc.hpp",
        "utils.hpp",
        "linkmgr_utils.hpp",
    ],
    deps = [
        "//nic/hal/lib:hal_lib_includes",
        "//nic/include:base_includes",
        "//nic:gen_proto_includes",
        "//nic/hal:hal_utils_includes",
        "//nic/utils/block_list:block_list",
        "//nic:google_includes",

        "//nic:libaapl",
        "//nic/linkmgr:linkmgr_includes",
        "//nic:liblinkmgr",

        "@sdk//obj:sdk_linkmgr",
        "@sdk//obj:sdk_logger",
        "@sdk//obj:sdk_thread",
        "@sdk//obj:sdk_twheel",
        "@sdk//obj:sdk_pal",
        "@sdk//obj:sdk_catalog",
        "@sdk//obj:sdk_utils",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_timerfd",
        "@sdk//obj:sdk_periodic",

        "//nic:halproto",
        "//nic:spdlog",
        "//nic:libprotobuf",
        "//nic:grpc",
        "//nic:libsknobs",
    ],
    alwayslink = 1,
)

########################################
# Shared libs and executable
########################################

# linkmgr executable
cc_binary(
    name = "linkmgr",
    srcs = [
        "main.cc",
    ],
    deps = [
        ":linkmgr_src",
    ],
    linkopts = [
        "-lpthread",
        "-pthread",
        "-lz",
        "-rdynamic",
        "-ldl",
        "-lm",
        "-lrt",
        "-lJudy",
    ],
)

# linkmgr shared lib
cc_binary(
    name = "liblinkmgr.so",
    linkshared = 1,
    deps = [
        ":linkmgr_libsrc",
    ],
)

# csr shared lib
cc_binary(
    name = "liblinkmgr_csr.so",
    linkshared = 1,
    deps = [
        ":linkmgrcsr",
    ],
)

