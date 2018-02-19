package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

#load("//nic/bazel:common_deps.bzl", "gl_deps_list")
#load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")

cc_library(
    name = "directmap",
    srcs = glob(["*.cc"]) + ["directmap_entry.hpp"],
    hdrs = ["directmap.hpp"],
    deps = [
            "//:sdk_headers",
            #"//lib/indexer:indexer",
            #"//lib/ht:ht",
            "//lib/p4pd:p4pd_api",
    ],
)
