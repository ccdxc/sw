package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "proxyplugin",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    copts = sdk_copts,
    deps = [
        "//nic:gen_includes",
        "//nic/hal:hal_src",
        "//nic:sdk_indexer",
        "//nic:sdk_slab",
        "//nic:sdk_ht",
        "//nic/utils/thread",
        "//nic/hal/tls:tls_api",
        "//nic/hal/pd/iris",
        "//nic/hal/lkl:lkl_api",
        "//nic:gen_proto_includes",
        "//nic/fte:fte_includes",
        #"//nic/include:base_includes",
        #"//nic/proto:types_proto",
        #"//nic/proto/agents:agents_proto",
        #"//nic/proto/hal:hal_proto",
        #"//nic/third-party/spdlog",
    ],
    alwayslink = 1,
)

cc_library(
    name = "proxyplugin_includes",
    hdrs = glob(["*.hpp"]),
)
