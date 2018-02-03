package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "acl_tcam",
    srcs = [
        "acl_tcam.cc",
        "acl_tcam.hpp",
        "tcam_entry.cc",
    ],
    hdrs = ["acl_tcam.hpp", "tcam_entry.hpp"],
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        #"//nic/hal/pd/utils/tcam:tcam_includes",
        "@sdk//obj:sdk_tcam",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic/utils/bitmap",
    ],
    copts = sdk_copts,
)

cc_library(
    name = "acl_tcam_includes",
    srcs = [],
    hdrs = ["acl_tcam.hpp"],
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic/utils/bitmap",
    ],
)

cc_test(
    name = "acl_tcam_test",
    srcs = ["test/acl_tcam_test.cc"],
    deps = [
            ":acl_tcam",
            "//nic/hal/pd/iris:pd_iris",
            "//nic/hal/pd/capri:capri",
            ] + gl_deps_list,
    linkopts = [
        "-lzmq",
     ],
    copts = sdk_copts,
)
