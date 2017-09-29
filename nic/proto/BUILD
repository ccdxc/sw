package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")
cc_proto_library(
   name = "types_proto",
   protos = ["types.proto"],
)

load("@org_pubref_rules_protobuf//python:rules.bzl", "py_proto_compile")
py_proto_compile(
   name = "types_proto_py",
   protos = ["types.proto"],
)
