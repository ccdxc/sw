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

# gflags
git_repository(
    name = "com_github_gflags_gflags",
    commit = "77592648e3f3be87d6c7123eb81cbad75f9aef5a",
    remote = "https://github.com/gflags/gflags.git",
)

bind(
    name = "gflags",
    actual = "@com_github_gflags_gflags//:gflags",
)

# python path used in swig
new_local_repository(
    name = "python_path",
    path = "/usr",
    build_file_content = """
cc_library(
    name = "python27-lib",
    hdrs = glob(["include/python2.7/*.h"]),
    includes = ["include/python2.7"],
    visibility = ["//visibility:public"]
)
    """
)
