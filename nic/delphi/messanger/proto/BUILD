package(default_visibility = ["//visibility:public"])

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")

filegroup(
    name = "protos",
    srcs = [
        "types.proto",
    ],
)

cc_proto_library(
  name = "types_cc_proto",
  protos = [":protos"],
  with_grpc = True,
  proto_deps  = [
      "//nic/delphi/proto/delphi:delphi_cc_proto",
 ],
 imports = [
     "/usr/local/include/",
     "/sw/nic/hal/third-party/google/include/",
 ]
)
