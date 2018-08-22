package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "sysmgr",
    srcs = glob(
        ["*.cc", "*.h"],
        exclude = ["*test.cc"],
    ),
    deps = [
        "//nic:spdlog",
    ],
    linkopts = [
        "-lpthread",
    ],
)

cc_test(
    name = "scheduler_test",
    srcs = glob(
        ["*.h", "*.cc"],
        exclude = ["main.cc"]
    ),
    linkopts = [
        "-lpthread",
    ],
    deps = [
        "//:gtest",
        "//nic:spdlog",
    ],
    timeout="short",
)