package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "lkl_api",
    srcs = [
        "lkl_api.cc",
        "lklshim.cc",
    ],
    hdrs = [
        "lkl_api.hpp",
        "lklshim.hpp",
    ],
    copts = [
        "-DCONFIG_AUTO_LKL_POSIX_HOST",
    ] + sdk_copts,
    includes = [
        "nic/third-party/lkl/export/include",
    ],
    deps = [
        "//nic:asic_includes",
        "//nic:gen_includes",
        "//nic:gen_proto_includes",
        "//nic:lkl_includes",
        "//nic:sdk_ht",
        "//nic:sdk_indexer",
        "//nic:sdk_slab",
        "//nic/asm/cpu-p4plus/include:cpu_p4plus_includes",
        "//nic/hal:hal_includes",
        "//nic/hal:hal_src",
        "//nic/hal/pd/capri:capri_includes",
        "//nic/hal/pd/common:pdcommon",
        "//nic/hal/pd/iris:iris_includes",
        "//nic/hal/periodic",
        "//nic/hal/plugins:plugins_includes",
        "//nic/hal/plugins/network:network_includes",
        "//nic/hal/plugins/proxy:proxyplugin_includes",
        "//nic/hal/tls:tls_api",
        "//nic/include:base_includes",
        "//nic/p4/include:p4_common_includes",
        "//nic/p4/nw/include:p4_nw_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/bm_allocator",
        "//nic/utils/host_mem:host_mem_includes",
        "//nic/utils/list:list_includes",
        "//nic/utils/print",
        "//nic/utils/thread",
    ],
)

cc_library(
    name = "lklshim_tls",
    srcs = [
        "lklshim_tls.cc",
    ],
    hdrs = [
        "lklshim.hpp",
        "lklshim_tls.hpp",
    ],
    copts = [
        "-DCONFIG_AUTO_LKL_POSIX_HOST",
    ] + sdk_copts,
    includes = [
        "nic/third-party/lkl/export/include",
    ],
    deps = [
        "//nic:asic_includes",
        "//nic:gen_includes",
        "//nic:gen_proto_includes",
        "//nic:lkl_includes",
        "//nic:sdk_ht",
        "//nic:sdk_indexer",
        "//nic:sdk_slab",
        "//nic/hal:hal_includes",
        "//nic/hal/pd/capri:capri_includes",
        "//nic/hal/pd/iris:iris_includes",
        "//nic/hal/periodic",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/block_list",
        "//nic/utils/bm_allocator",
        "//nic/utils/catalog",
        "//nic/utils/host_mem:host_mem_includes",
        "//nic/utils/list:list_includes",
        "//nic/utils/print",
        "//nic/utils/thread",
    ],
)
