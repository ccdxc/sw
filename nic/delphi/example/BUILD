package(default_visibility = ["//visibility:public"])

load("//nic/delphi/compiler:delphi.bzl", "delphi_compile")

# compile the proto file
delphi_compile(
    name = "example_gen",
    srcs = [
      'example.proto',
      'example_stats.proto',
    ],
)

# compile the service and reactor
cc_library(
    name = "example_lib",
    srcs = [
    ] + glob(["*.cc"], exclude = ["*_test.cc", "*_main.cc"],) +  glob(["*.hpp"]),
    deps = [
        "//nic/delphi/sdk",
        ":example_gen",
    ],
)

# Compile the example binary
cc_binary(
    name = "example",
    srcs = [
        "example_main.cc"
    ],
    deps = [
        ":example_lib",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    linkopts = [
        "-lpthread",
        "-lm",
    ],
)

# run the tests
cc_test(
    name = "example_test",
    srcs = [
        "example_test.cc",
    ],
    linkopts = [
        "-lpthread",
        "-lm",
    ],
    deps = [
        "//:gtest",
        ":example_lib",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    timeout="short",
)
