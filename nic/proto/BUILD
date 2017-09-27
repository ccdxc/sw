package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

# load("@org_pubref_rules_protobuf//protobuf:rules.bzl", "proto_language", "proto_compile")
# 
# proto_compile(
#         name = "types_proto",
#         protos = ["types.proto"],
#         langs = [
#                 "@org_pubref_rules_protobuf//cpp",
#                 "@org_pubref_rules_protobuf//python",
#                 ],
#         verbose = 1,
#         with_grpc = True,
#         )
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
