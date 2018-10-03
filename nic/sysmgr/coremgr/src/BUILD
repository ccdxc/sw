package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "coremgr",
    srcs = glob(
        ["*.cpp", "*.hpp"],
        exclude = ["*test.cpp"],
    ),
    deps = [
        "//nic:spdlog",
    ],
    linkopts = [
        "-lz",
        "-lpthread",
    ],
)
