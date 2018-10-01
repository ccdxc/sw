# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "fte",
    srcs = glob(["*.cc", "acl/*.cc", "utils/*.cc"]),
    hdrs = glob(["*.hpp", "acl/*.hpp", "utils/*.hpp"]),
    copts = sdk_copts,
    deps = [
        "//nic/include:base_includes",
        "//nic/hal/pd:hal_pd_includes",
        "//nic/asm/cpu-p4plus/include:cpu_p4plus_includes",
        "//nic:spdlog",
        "//nic/utils/agent_api:agent_api",
        "//nic/hal/plugins/app_redir:app_redir_includes",
        "//nic/hal/plugins/cfg/lif:lif_includes",
        "//nic/hal/iris/datapath/p4/include:p4_iris_includes",
        "//nic/p4/common:p4_common_includes",
        "//nic/hal/third-party/packet_parser"
    ],
    alwayslink = 1,
)

cc_library(
    name = "fte_core_includes",
    hdrs = [
        "fte_core.hpp",
    ],
)

cc_library(
    name = "fte_includes",
    hdrs = [
        "fte.hpp",
        "fte_ctx.hpp",
        "acl/acl.hpp",
        "acl/ref.hpp",
        "utils/packet_utils.hpp"
    ],
    deps = [
        "//nic/fte:fte_core_includes",
        "//nic/utils/agent_api:agent_api",
    ],
)

cc_test(
    name = "fte_test",
    srcs = glob(["test/*.cc", "test/*.hpp"]),
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    deps = [ "//nic/utils/agent_api:agent_api",
             "//nic/hal/plugins/sfw:sfw_pkt_utils",
             "//nic/hal/plugins/sfw:sfw",
             "//nic/e2etests:e2etest-lib",
             "//nic:libtins",
             "//nic/hal/plugins/alg_utils:alg_utils",
           ] + gl_deps_list + test_deps_list,
    copts = sdk_copts,
    tags = ["gtests"],
)

cc_library(
    name = "fte_base_test",
    srcs = [ "test/fte_base_test.cc" ],
    hdrs = [ "test/fte_base_test.hpp" ],
    deps = [ "//nic/utils/agent_api:agent_api",
             "//nic/hal/plugins/sfw:sfw_pkt_utils",
             "//nic/e2etests:e2etest-lib",
           ] + gl_deps_list + test_deps_list,
    copts = sdk_copts,
    tags = ["gtests"],
)
