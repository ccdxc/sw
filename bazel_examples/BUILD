package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "bm_allocator",
    srcs = ["bm_allocator.cc"],
    hdrs = ["bm_allocator.hpp"],
)

cc_test(
    name = "bm_allocator_test",
    srcs = ["bm_allocator_test.cc"],
    deps = [
        ":bm_allocator",
        "//:gtest",
    ],
)

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")
load("@org_pubref_rules_protobuf//cpp:rules.bzl", "GRPC_COMPILE_DEPS")
cc_proto_library(
    name = "sample_proto",
    protos = ["sample.proto"],
)

cc_library(
    name = "sample",
    srcs = ["sample.cc"],
    deps = [
        "//bazel_examples:sample_proto",
    ] + GRPC_COMPILE_DEPS,
)

load("@org_pubref_rules_protobuf//python:rules.bzl", "py_proto_compile")
py_proto_compile(
    name = "sample_proto_py",
    protos = ["sample.proto"],
)
