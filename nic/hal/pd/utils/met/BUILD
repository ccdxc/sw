package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")

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
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        "//nic/hal/pd/capri",
        "//nic/hal/pd/control",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/indexer",
        "//nic/utils/thread",
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
    deps = gl_deps_list,
)
