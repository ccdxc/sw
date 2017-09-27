package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "shmseg",
    srcs = [
        "shmseg.cc",
        "shmseg.hpp",
    ],
    hdrs = ["shmseg.hpp"],
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
    ],
)
cc_test(
    name = "shmseg_test",
    srcs = ["test/shmseg_test.cc"],
    deps = [
        # Utils
        ":shmseg",
        "//nic/utils/trace",
        # External
        "//:gtest",
    ],
    linkopts = ["-lrt"],
    copts = [
        "-Wno-write-strings",
        ],
)
