package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license


# Target which gets all symbols from bm_allocator
cc_binary(
        name = "libhost_mem.so",
        linkshared = 1,
        deps = [
                "//nic/utils/host_mem:host_mem_src"
        ],
)

cc_library(
    name = "host_mem_src",
    srcs = [
        "host_mem.cc",
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
        ":host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",

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
