package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "tls_api",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    copts = sdk_copts,
    deps = [
        "//nic:openssl",
        "//nic/hal:hal_includes",
        "//nic/hal/lkl:lklshim_tls",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic:boost",
        "//nic:google_includes",
        "//nic/hal/pd/common:pdcommon",
        "@sdk//obj:sdk_indexer",
        "@sdk//obj:sdk_slab",
        "//nic:gen_proto_includes",
        "//nic:p4pd"
    ],
)
