package(default_visibility = ["//visibility:public"])

# run the tests
cc_test(
    name = "upgrade_manager_test",
    srcs = glob(["*.cc"]),
    linkopts = [
        "-lpthread",
    ],
    deps = [
        "//:gtest",
        "//nic/upgrade_manager/lib:upgrade_lib",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    timeout="short",
)
