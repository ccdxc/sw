package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_test(
    name = "rw_tbl_test",
    srcs = glob(["rw_tbl_lib/*.cc"]),
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lrt",
    ],
    deps = gl_deps_list + [
            "//nic/hal/pd/iris:pd_iris",
            "//nic/hal/pd/capri:capri_includes",
            "//nic/hal/pd:pdcommon",
    		"//nic/hal/pd/utils/p4pd_mock:p4pd_mock",
            "//nic/hal/pd:hal_pd_includes",
    ] + test_deps_list,
    copts = sdk_copts,
)

cc_test(
    name = "tnnl_rw_tbl_test",
    srcs = glob(["tnnl_rw_tbl_lib/*.cc"]),
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lrt",
    ],
    deps =  gl_deps_list + [
            "//nic/hal/pd/iris:pd_iris",
            "//nic/hal/pd/capri:capri",
            "//nic/hal/pd:pdcommon",
            "//nic/hal/pd/utils/p4pd_mock:p4pd_mock",
            "//nic/hal/pd:hal_pd_includes",
    ] + test_deps_list,
    copts = sdk_copts,
)
