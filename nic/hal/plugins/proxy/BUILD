package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "proxyplugin",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    copts = sdk_copts,
    deps = [
        #"//nic:gen_includes",
        "//nic:google_includes",
        "//nic/hal:hal_src",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_indexer",
        "@sdk//obj:sdk_slab",
        "//nic/hal/tls:tls_api",
        #"//nic/hal/pd/iris",
        "//nic/hal/lkl:lkl_api",
        "//nic:gen_proto_includes",
        "//nic/fte:fte_includes",
    ],
    alwayslink = 1,
)

cc_library(
    name = "proxyplugin_includes",
    hdrs = glob(["*.hpp"]),
)
