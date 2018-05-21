package(default_visibility = ["//visibility:public"])

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")
load("//nic/delphi/compiler:delphi.bzl", "delphi_compile")

# compile the proto file
delphi_compile(
    name = "hal_svc_gen",
    srcs = [
        'types.proto',
        'kh.proto',
        'eplearn.proto',
        'endpoint.proto',
        'interface.proto',
        'l2segment.proto',
        'nwsec.proto',
    ],
)
