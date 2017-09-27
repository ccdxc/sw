package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "host_mem",
    srcs = [
        "host_mem.cc",
        "bm_allocator.cc",
        "c_if.cc",
        "c_if.h",
        "host_mem.hpp",
        "params.hpp",
    ],
    hdrs = [
        "host_mem.hpp",
        "params.hpp",
    ],
    deps = [
        "//nic/utils/bm_allocator:bm_allocator",
    ],
)

cc_test(
     name = "host_mem_test",
     srcs = ["test/host_mem_test.cc"],
     deps = [
        ":host_mem",
        # "//nic/utils/trace:trace",

        # External 
        "//:gtest",
     ],
)

# Bharat: Cleanup. 
cc_library(
    name = "host_mem_includes",
    srcs = [],
    hdrs = [
        "host_mem.hpp",
        "params.hpp",
    ],
)
