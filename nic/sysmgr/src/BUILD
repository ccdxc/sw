package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "sysmgr",
    srcs = glob(
        ["*.cpp", "*.hpp"],
        exclude = ["*test.cpp"],
    ),
    deps = [
        "//nic/delphi/sdk",
        "//nic/sysmgr/proto:sysmgr_gen",
        "//nic:spdlog",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    linkopts = [
        "-lpthread",
    ],
)

cc_binary(
    name = "sysmgr_test",
    srcs = glob(
        ["*.cpp", "*.hpp"],
        exclude = ["*test.cpp"],
    ),
    deps = [
        "//nic/delphi/sdk",
        "//nic/sysmgr/proto:sysmgr_gen",
        "//nic:spdlog",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    linkopts = [
        "-lpthread",
    ],
    defines = [
        "SYSMGR_TEST",
    ]
)

cc_test(
    name = "scheduler_test",
    srcs = glob(
        ["*.cpp", "*.hpp"],
        exclude = ["main.cpp"]
    ),
    linkopts = [
        "-lpthread",
    ],
    deps = [
        "//:gtest",
        "//nic/delphi/sdk",
        "//nic/sysmgr/proto:sysmgr_gen",
        "//nic:spdlog",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    timeout="short",
)

cc_test(
    name = "watchdog_test",
    srcs = [
        "watchdog.cpp",
        "watchdog.hpp",
        "watchdog_test.cpp",
    ],
    linkopts = [
        "-lpthread",
    ],
    deps = [
        "//:gtest",
    ],
    timeout="short",
)