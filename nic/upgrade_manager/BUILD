package(default_visibility = ["//visibility:public"])

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")
load("//nic/delphi/compiler:delphi.bzl", "delphi_compile")

# compile the proto file
delphi_compile(
    name = "upgrade_gen",
    srcs = [ 'proto/upgrade.proto' ],
)

# compile the service and reactor
cc_library(
    name = "upgrade_lib",
    srcs = [
        "lib/upgrade.cc",
        "lib/upgrade.hpp",
    ],
    hdrs = [
        "lib/upgrade.hpp",
    ],
    deps = [
        "//nic/delphi/sdk",
        ":upgrade_gen",
    ],
)

# Compile the upgrade binary
cc_binary(
    name = "upgrade",
    srcs = [
        "src/upgrade_main.cc"
    ],
    deps = [
        ":upgrade_lib",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
)

# run the tests
cc_test(
    name = "upgrade_test",
    srcs = [
        "test/upgrade_test.cc",
    ],
    linkopts = [
        "-lpthread",
    ],
    deps = [
        "//:gtest",
        ":upgrade_lib",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    timeout="short",
)

