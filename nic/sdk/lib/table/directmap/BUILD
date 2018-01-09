package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

#load("//nic/bazel:common_deps.bzl", "gl_deps_list")
#load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")

cc_library(
    name = "directmap",
    srcs = glob(["*.cc"]) + ["directmap_entry.hpp"],
    hdrs = ["directmap.hpp"],
    deps = [
        #"//lib/ht:ht",
        #"@sdk//obj:sdk_ht",
        #"@sdk//obj:sdk_indexer",
        #"@sdk//obj:sdk_slab",
        #"//nic/hal/pd:hal_pd_includes",
        #"//nic/hal/pd/iris:iris_includes",
        #"//nic/include:base_includes",
        #"//nic/third-party/spdlog",
        #"//nic/hal/pd/capri:capri_includes",
    ],
)

#cc_library(
#    name = "directmap_includes",
#    srcs = [],
#    hdrs = [
#        "directmap.hpp",
#        "directmap_entry.hpp",
#    ],
#)

#cc_test(
#    name = "directmap_test",
#    srcs = ["test/directmap_test.cc"],
#    linkopts = gl_linkopts_list,
#    deps = gl_deps_list,
#    copts = sdk_copts,
#)
