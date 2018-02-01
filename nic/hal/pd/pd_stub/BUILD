package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "pd_stub",
    srcs = glob(["*.cc"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/hal:hal_hdrs",
        "//nic:spdlog",
        "//nic/utils/block_list",
        "//nic/hal/periodic",
        "//nic:halproto",
        "//nic:gen_proto_includes",
        #"//nic/hal/pd/iris:iris_includes", # this exists only because session.hpp include p4pd header files !!
        "//nic/hal/pd:hal_pd_includes",
    ],
    alwayslink = 1,
)
