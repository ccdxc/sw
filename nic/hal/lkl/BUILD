package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

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
    deps = ["//nic/include:base_includes",
            "//nic/hal:hal_includes",
            "//nic/utils/list:list_includes",
            "//nic/utils/bm_allocator:bm_allocator",
            "//nic/utils/thread:thread",
            "//nic/utils/print:print",
            "//nic/utils/slab:slab",
            "//nic/utils/indexer:indexer",
            "//nic/utils/ht:ht",
            "//nic/proto:types_proto",
            "//nic/proto/hal:hal_proto",
            "//nic:gen_includes",
            "//nic/hal/pd/capri:capri_includes",
            "//nic/hal/pd/iris:iris_includes",
            "//nic/utils/host_mem:host_mem_includes",
            # "//nic/utils/host_mem:host_mem",
            "//nic/third-party/spdlog:spdlog",
            "//nic:lkl_includes",
        "//nic/hal/periodic",
        "//nic:asic_includes",
        "//nic/hal:hal_src",
        "//nic/hal/pd/common:pdcommon",
        "//nic/p4/include:p4_common_includes",
        "//nic/p4/nw/include:p4_nw_includes",
        "//nic/asm/cpu-p4plus/include:cpu_p4plus_includes",
        "//nic/hal/plugins:plugins_includes",
        "//nic/hal/plugins/network:network_includes",
        "//nic/hal/plugins/proxy:proxyplugin_includes",
            ],
    includes = [
            "nic/third-party/lkl/export/include",
            ],
    copts = [
            "-DCONFIG_AUTO_LKL_POSIX_HOST",
            ],
)
