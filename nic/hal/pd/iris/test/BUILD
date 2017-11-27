package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")

cc_test(
    name = "rw_tbl_test",
    srcs = glob(["rw_tbl_lib/*.cc"]),
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lprotobuf",
    ],
    deps = gl_deps_list + [
    		"//nic/hal/pd/utils/p4pd_mock:p4pd_mock",
    ],
)

cc_test(
    name = "tnnl_rw_tbl_test",
    srcs = glob(["tnnl_rw_tbl_lib/*.cc"]),
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lprotobuf",
    ],
    deps =  gl_deps_list + [
		"//nic/hal/pd/utils/p4pd_mock:p4pd_mock",
    ],
)
