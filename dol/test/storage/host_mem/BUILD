package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

# This dir will be removed once main repo moves to bazel
# and we can directly add 'deps' for nic/utils/host_mem:host_mem

cc_library(
    name= "host_mem",
    srcs = [
        "bm_allocator.cc",
        "bm_allocator.hpp",
        "c_if.cc",
        "c_if.h",
        "host_mem.cc",
        "host_mem.hpp",
        "params.hpp",
    ],
    hdrs = [
        "c_if.h",
        "host_mem.hpp",
        "params.hpp",
    ],
)
