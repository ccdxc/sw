package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "flow",
    srcs = [
        "flow.cc",
        "flow.hpp",
        "flow_entry.cc",
        "flow_entry.hpp",
        "flow_hint_group.cc",
        "flow_hint_group.hpp",
        "flow_spine_entry.cc",
        "flow_spine_entry.hpp",
        "flow_table_entry.cc",
        "flow_table_entry.hpp",
    ],
    hdrs = [
        "flow.hpp",
        "flow_entry.hpp",
        "flow_hint_group.hpp",
        "flow_spine_entry.hpp",
        "flow_table_entry.hpp",
    ],
    copts = sdk_copts,
    deps = [
        "//nic:gen_includes",
        "@sdk//obj:sdk_indexer",
        "//nic/hal/pd:hal_pd_includes",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic:boost",
    ]
)

cc_library(
    name = "flow_includes",
    srcs = [],
    hdrs = [
        "flow.hpp",
        "flow_entry.hpp",
        "flow_hint_group.hpp",
        "flow_spine_entry.hpp",
        "flow_table_entry.hpp",
    ],
)

cc_test(
    name = "flow_test",
    srcs = ["test/flow_test.cc"],
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
    ],
    deps = [] + gl_deps_list,
    copts = sdk_copts,
)
