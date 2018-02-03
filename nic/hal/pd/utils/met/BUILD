package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "met",
    srcs = [
        "met.cc",
        "met.hpp",
        "repl_entry.cc",
        "repl_entry.hpp",
        "repl_list.cc",
        "repl_list.hpp",
        "repl_table_entry.cc",
        "repl_table_entry.hpp",
    ],
    hdrs = [
        "met.hpp",
        "repl_entry.hpp",
        "repl_list.hpp",
        "repl_table_entry.hpp",
    ],
    copts = sdk_copts,
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        "//nic/hal/pd/capri",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "@sdk//obj:sdk_indexer",
        "@sdk//obj:sdk_pal",
    ],
)

cc_library(
    name = "met_includes",
    srcs = [],
    hdrs = [
        "met.hpp",
        "repl_entry.hpp",
        "repl_list.hpp",
        "repl_table_entry.hpp",
    ],
)

cc_test(
    name = "met_test",
    srcs = ["test/met_test.cc"],
    linkopts = gl_linkopts_list,
    deps = [
            "//nic/hal/pd/iris:pd_iris",
            "//nic/hal/pd/capri:capri",
            ] + gl_deps_list,
    copts = sdk_copts,
)
