# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "fte",
    srcs = glob(["*.cc", "acl/*.cc"]),
    hdrs = glob(["*.hpp", "acl/*.hpp"]),
    copts = sdk_copts,
    deps = [
        "//nic:gen_proto_includes",
        "//nic:grpc",
        "//nic:halproto",
        "//nic/hal:hal_src",
        "//nic:hal_svc_gen",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic/hal/plugins/app_redir:app_redir_includes",
        "//nic/utils/agent_api:agent_api",
        "//nic/p4/iris/include:p4_iris_includes",
    ],
    alwayslink = 1,
)

cc_library(
    name = "fte_includes",
    hdrs = [
        "fte.hpp",
        "fte_ctx.hpp",
        "acl/acl.hpp",
        "acl/ref.hpp"
    ],
    deps = [ "//nic/utils/agent_api:agent_api",
           ],
)

cc_test(
    name = "fte_test",
    srcs = glob(["test/*.cc", "test/*.hpp"]),
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    deps = [ "//nic/utils/agent_api:agent_api", 
           ] + gl_deps_list + test_deps_list,
    copts = sdk_copts,
)

cc_library(
    name = "fte_base_test",
    srcs = [ "test/fte_base_test.cc" ],
    hdrs = [ "test/fte_base_test.hpp" ],
    deps = [ "//nic/utils/agent_api:agent_api",
           ] + gl_deps_list + test_deps_list,
    copts = sdk_copts,
    tags = ["gtests"],
)
