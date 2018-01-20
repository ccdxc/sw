# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

# Logger
cc_library(
    name = "sdk_logger",
    srcs = [
        "liblogger.so",
    ],
    deps = [
        "//:sdk_headers",
    ],
)

# List
cc_library(
    name = "sdk_list",
    srcs = [
    ],
    deps = [
        "//:sdk_headers",
    ],
)

# Slab 
cc_library(
    name = "sdk_slab",
    srcs = [
        "libslab.so",
    ],
    deps = [
        "//:sdk_headers",
    ],
)

# Hash Table (HT) 
cc_library(
    name = "sdk_ht",
    srcs = [
        "libht.so",
    ],
    deps = [
        "//:sdk_headers",
    ],
)

# Indexer
cc_library(
    name = "sdk_indexer",
    srcs = [
        "libindexer.so",
    ],
    deps = [
        "//:sdk_headers",
    ],
)

# PAL
cc_library(
    name = "sdk_pal",
    srcs = [
        "libpal.so",
    ],
    deps = [
        "//:sdk_headers",
    ],
)

# Directmap/Index Table Management Library
cc_library(
    name = "sdk_directmap",
    srcs = [
        "libdirectmap.so",
    ],
    deps = [
        "//lib/table/directmap:directmap",
    ],
)

# Tcam Table Management Library
cc_library(
    name = "sdk_tcam",
    srcs = [
        "libtcam.so",
    ],
    deps = [
        "//lib/table/tcam:tcam",
    ],
)

# Hash Table Management Library
cc_library(
    name = "sdk_hash",
    srcs = [
        "libhash.so",
    ],
    deps = [
        "//lib/table/hash:hash",
    ],
)

# catalog library
cc_library(
    name = "sdk_catalog",
    srcs = [
        "libcatalog.so",
    ],
    deps = [
        "//:sdk_headers",
    ],
)

# thread library
cc_library(
    name = "sdk_thread",
    srcs = [
        "libthread.so",
    ],
    deps = [
        "//:sdk_headers",
    ],
)

# twheel library
cc_library(
    name = "sdk_twheel",
    srcs = [
        "libtwheel.so",
    ],
    deps = [
        "//:sdk_headers",
    ],
)

# timerfd library
cc_library(
    name = "sdk_timerfd",
    srcs = [
        "libtimerfd.so",
    ],
    deps = [
        "//:sdk_headers",
    ],
)
