package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

# proto_compile(
#         name = "hal_proto",
#         protos = [":protos"],
#         langs = [
#                 "//cpp",
#                 "//python",
#                 ],
#         proto_deps = [
#                 "//nic/proto:types_proto",
#                 ],
#         verbose = 1,
#         with_grpc = True,
#         )



load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")
cc_proto_library(
   name = "hal_proto",
   protos = [
   ] + glob(["*.proto"]),
   proto_deps = [
       "//nic/proto:types_proto",
   ],
   with_grpc = True,
)

load("@org_pubref_rules_protobuf//python:rules.bzl", "py_proto_compile")
py_proto_compile(
    name = "hal_proto_py",
    protos = [
    ] + glob(["*.proto"]),
    deps = [
        "//nic/proto:types_proto_py",
    ],
    with_grpc = True,
)
