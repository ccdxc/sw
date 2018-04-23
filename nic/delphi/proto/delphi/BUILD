package(default_visibility = ["//visibility:public"])

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")

filegroup(
    name = "delphi_protos",
    srcs = [
        "delphi.proto",
    ],
)

cc_proto_library(
  name = "delphi_cc_proto",
  protos = [":delphi_protos"],
  with_grpc = True,
  imports = [
      "/usr/local/include/",
      "/sw/nic/hal/third-party/google/include/",
  ]
)
