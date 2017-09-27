package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")
cc_proto_library(
   name = "agents_proto",
   protos = [
           ] + glob(["*.proto"]),
   proto_deps = [
       "//nic/proto:types_proto",
   ],
   with_grpc = True,
)

load("@org_pubref_rules_protobuf//python:rules.bzl", "py_proto_compile")
py_proto_compile(
    name = "agents_proto_py",
    protos = [
    ] + glob(["*.proto"]),
    deps = [
        "//nic/proto:types_proto_py",
    ],
    with_grpc = True,
)
# load("@org_pubref_rules_protobuf//python:rules.bzl", "py_proto_compile")
# py_proto_compile(
#    name = "hal_proto_py",
#    protos = [
#        "acl.proto",
#        "crypto_keys.proto",
#        "descriptor_aol.proto",
#        "endpoint.proto",
#        "interface.proto",
#        "ipseccb.proto",
#        "l2segment.proto",
#        "l4lb.proto",
#        "multicast.proto",
#        "nic.proto",
#        "nw.proto",
#        "nwsec.proto",
#        "proxy.proto",
#        "qos.proto",
#        "session.proto",
#        "tcp_proxy_cb.proto",
#        "telemetry.proto",
#        "tenant.proto",
#        "tls_proxy_cb.proto",
#        "wring.proto",
#    ],
#    proto_deps = [
#        "//nic/proto:types_proto",
#    ],
#    with_grpc = True,
# )
