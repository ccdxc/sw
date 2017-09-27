package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "bitmap",
    srcs = ["bitmap.cc", "bitmap.hpp"],
    hdrs = ["bitmap.hpp"],
    deps = ["//nic/include:base_includes", "//nic/third-party/spdlog:spdlog", "//nic/hal:hal_includes"],
)

cc_test(
     name = "bitmap_test",
     srcs = ["test/bitmap_test.cc"],
     deps = [
        ":bitmap",
        # "//nic/utils/trace:trace",

        # External 
        "//:gtest",
     ],
)
