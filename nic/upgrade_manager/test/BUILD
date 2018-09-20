package(default_visibility = ["//visibility:public"])

# run the tests
cc_test(
    name = "upgrade_manager_test",
    srcs = glob(["upgrade_manager_test.cc"]),
    linkopts = [
        "-lpthread",
        "-lm",
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

cc_test(
    name = "upgrade_sdk_test",
    srcs = glob(["upgrade_sdkclib_test.cc"]),
    linkopts = [
        "-lpthread",
        "-lm",
    ],
    deps = [
        "//:gtest",
        "//nic/upgrade_manager/export/upgcsdk:upgrade_app_lib",
        "//nic/upgrade_manager/export/upgcsdk:upgrade_app_lib_includes",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    timeout="short",
)
