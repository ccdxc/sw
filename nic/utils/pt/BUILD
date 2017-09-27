package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pt",
    srcs = [
        "pt.cc",
        "pt.hpp",
    ],
    hdrs = ["pt.hpp"],
    deps = [
        "//nic/hal:hal_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/thread:thread",
        "//nic/utils/slab:slab",
        "//nic/hal/periodic:periodic",
        "//nic/utils/print:print",
    ],
)

cc_test(
    name = "pt_test",
    srcs = ["test/pt_test.cc"],
    deps = [
        # Utils
        ":pt",
        "//nic/utils/trace",
        "//nic/hal/test:hal_mock",
        # External
        "//:gtest",
    ],
)
