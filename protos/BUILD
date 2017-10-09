# Indirection for all external dependencies.
package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")
cc_proto_library(
    name = "gogo_proto",
    protos = [
        "github/gogo/protobuf/gogoproto/gogo.proto",
        ],
    imports = ["/usr/local/include"],
)

load("@org_pubref_rules_protobuf//python:rules.bzl", "py_proto_compile")
py_proto_compile(
    name = "gogo_proto_py",
    protos = [
        "github/gogo/protobuf/gogoproto/gogo.proto",
        ],
    imports = ["/usr/local/include"],
)
