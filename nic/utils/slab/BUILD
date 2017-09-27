package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "slab",
    srcs = ["slab.cc", "slab.hpp"],
    hdrs = ["slab.hpp"],
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog:spdlog",
        "//nic/utils/list:list_includes",
    ],
)

cc_test(
    name = "slab_test",
    srcs = ["test/slab_test.cc"],
    deps = [
        # Utils
        ":slab",
        "//nic/utils/trace",
        "//nic/hal/periodic:periodic",
        "//nic/hal/test:hal_mock",
        # External
        "//:gtest",
    ],
)
