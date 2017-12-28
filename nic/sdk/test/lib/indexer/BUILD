package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

# Indexer Test
cc_test(
    name = "indexer_test",
    srcs = ["indexer_test.cc"],
    deps = [
        "//:sdk_headers",
        "//obj:sdk_indexer",
        "//obj:sdk_logger",
        # Utils
        #":indexer",
        # External
        "//:gtest",
    ],
)
