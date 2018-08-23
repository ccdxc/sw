package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "lkl_api",
    srcs = [
        "lkl_api.cc",
        "lklshim.cc",
        "lklshim_v6.cc",
    ],
    hdrs = [
        "lkl_api.hpp",
        "lklshim.hpp",
    ],
    copts = [
        "-DCONFIG_AUTO_LKL_POSIX_HOST",
    ] + sdk_copts,
    deps = [
        "//nic:asic_includes",
        "//nic:gen_proto_includes",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_indexer",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_list",
        "//nic/asm/cpu-p4plus/include:cpu_p4plus_includes",
        "//nic/hal:hal_includes",
        "//nic/hal/plugins/cfg/nw:nw_includes",
        "//nic/hal/plugins/cfg/aclqos:aclqos_includes",
        "//nic/hal/plugins/sfw:firewall_includes",
        "//nic/hal/pd:pdcommon",
        "@sdk//obj:sdk_periodic",
        "//nic/hal/core:core_includes",
        "//nic/hal/plugins/network:network_includes",
        "//nic/hal/plugins/proxy:proxyplugin_includes",
        "//nic/hal/tls:tls_api",
        "//nic/include:base_includes",
        "//nic/p4/include:p4_common_includes",
        "//nic/hal/iris/datapath/p4/include:p4_iris_includes",
        "//nic/p4/common:p4_common_includes",
        "//nic:spdlog",
        "//nic:boost",
        "//nic:lkl_includes",
        "//nic/utils/bm_allocator",
        "//nic/utils/host_mem:host_mem_includes",
        "//nic/utils/print",
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
    deps = [
        "//nic:asic_includes",
        "//nic:gen_proto_includes",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_indexer",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_list",
        "//nic/hal:hal_includes",
        "//nic/hal/plugins/cfg/nw:nw_includes",
        "//nic/hal/plugins/cfg/lif:lif_includes",
        "//nic/hal/plugins/cfg/aclqos:aclqos_includes",
        "//nic/hal/plugins/sfw:firewall_includes",
        "//nic/hal/lib:hal_lib_includes",
        "@sdk//obj:sdk_periodic",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic:boost",
        "//nic:google_includes",
        "//nic:lkl_includes",
        "//nic/utils/block_list",
        "//nic/utils/bm_allocator",
        "//nic/utils/host_mem:host_mem_includes",
        "//nic/utils/print",
        "//nic/utils/agent_api",
        "//nic/fte",
    ],
)
