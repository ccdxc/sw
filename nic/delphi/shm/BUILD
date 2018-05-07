package(default_visibility = ["//visibility:public"])

cc_library(
    name = "shm",
    srcs = [
    ] + glob(["*.cc"], exclude = ["*_test.cc", "*_main.cc"],) +  glob(["*.hpp"]),
    hdrs = [
        "shm.hpp",
        "slab_mgr.hpp",
        "atomic.hpp",
    ],
    include_prefix = "nic/delphi/shm",
    deps = [
        "//nic/delphi/utils",
    ],
)

cc_test(
    name = "slab_mgr_test",
    srcs = [
        "slab_mgr_test.cc",
    ],
    linkopts = [
        "-lpthread",
        "-lrt",
    ],
    deps = [
        "//:gtest",
        ":shm",
        "//nic/delphi/utils",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    timeout="short",
)

cc_test(
    name = "shm_test",
    srcs = [
        "shm_test.cc",
    ],
    linkopts = [
        "-lpthread",
        "-lrt",
    ],
    deps = [
        "//:gtest",
        ":shm",
        "//nic/delphi/utils",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    timeout="short",
)

cc_test(
    name = "kvstore_test",
    srcs = [
        "kvstore_test.cc",
    ],
    linkopts = [
        "-lpthread",
        "-lrt",
    ],
    deps = [
        "//:gtest",
        ":shm",
        "//nic/delphi/utils",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    timeout="short",
)

cc_binary(
    name = "shm_test_pgm",
    srcs = [
        "shm_test_main.cc",
    ],
    linkopts = [
        "-lpthread",
        "-lrt",
    ],
    deps = [
        ":shm",
        "//nic/delphi/utils",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
)
