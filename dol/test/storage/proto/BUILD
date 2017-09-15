package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

# This dir will be removed once main repo moves to bazel
# and we can directly add 'deps' for nic/proto/hal:hal_proto

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")

cc_proto_library(
   name = "hal_proto",
   protos = [
       "types.proto",
       "qos.proto",
       "internal.proto",
       "l2segment.proto",
       "interface.proto",
   ],
   with_grpc = True,
)
