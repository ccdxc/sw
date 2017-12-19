package(default_visibility = ["//visibility:public"])
load("//nic/bazel:common_deps.bzl", "sdk_copts")

licenses(["notice"])  # MIT license

cc_library(
    name = "app_redir",
    srcs = [
        "app_redir.cc",
        "app_redir_plugin.cc",
        "app_redir_appid.cc",
        "app_redir_scanner.cc",
        "app_redir_cb_ops.cc",
    ],
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic:gen_includes",
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/third-party/spdlog",
        "//nic:gen_proto_includes",
        "//nic:snort3",
    ],
    copts = sdk_copts,
    alwayslink = 1,
)

cc_library(
    name = "app_redir_mock",
    srcs = [
        "app_redir.cc",
        "app_redir_plugin.cc",
        "app_redir_appid.cc",
        "app_redir_scanner_mock.cc",
        "app_redir_cb_ops.cc",
    ],
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic:gen_includes",
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/third-party/spdlog",
        "//nic:gen_proto_includes",
    ],
    copts = sdk_copts,
    alwayslink = 1,
)

cc_library(
    name = "app_redir_includes",
    hdrs = [
        "app_redir_ctx.hpp"
    ],
)
