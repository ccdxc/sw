new_git_repository(
    name = "gtest",
    build_file = "BUILD.gtest",
    remote = "https://github.com/google/googletest",
    tag = "release-1.8.0",
)

# Protobuf with gRPC support
git_repository(
  name = "org_pubref_rules_protobuf",
  remote = "https://github.com/pubref/rules_protobuf",
  tag = "v0.7.1",
)
load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cpp_proto_repositories")
cpp_proto_repositories()
load("@org_pubref_rules_protobuf//python:rules.bzl", "py_proto_repositories")
py_proto_repositories()
