package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "common_includes",
    hdrs = glob(["*.hpp"]),
)

cc_binary(
    name = "nic_proxy-e2etest_ethtx",
    srcs = [
        "test/eth_tx.cc",
    ],
    linkopts = [
        "-lrt",
    ],

    deps = [
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "@sdk//obj:sdk_logger",

        # PD
        "//nic/model_sim:model_client",
        "//nic:spdlog",
    ],
)

cc_binary(
    name = "nic_proxy-e2etest_ethrx",
    srcs = [
        "test/eth_rx.cc",
    ],
    linkopts = [
        "-lrt",
    ],
    deps = [
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "@sdk//obj:sdk_logger",

        # PD
        "//nic/model_sim:model_client",
    ],
)

cc_binary(
    name = "nic_proxy-e2etest_tls-client",
    srcs = [
        "utls/tls-client.cc",
    ],

    linkopts = [
        "-lrt",
    ],


    deps = [
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic:openssl-native",
        "@sdk//obj:sdk_logger",

        # PD
        "//nic/model_sim:model_client",
    ],
)
cc_binary(
    name = "nic_proxy-e2etest_tcp-client",
    srcs = [
        "utls/tcp-client.cc",
    ],

    linkopts = [
        "-lrt",
    ],
    deps = [
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "@sdk//obj:sdk_logger",

        # PD
        "//nic/model_sim:model_client",
    ],
)

cc_binary(
    name = "nic_proxy-v6e2etest_tcp-client",
    srcs = [
        "utls/tcp-client-v6.cc",
    ],
    linkopts = [
        "-lrt",
    ],
    deps = [
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "@sdk//obj:sdk_logger",

        # PD
        "//nic/model_sim:model_client",
    ],
)

cc_binary(
    name = "nic_proxy-e2etest_tls-server",
    srcs = [
        "utls/tls-server.cc",
    ],
    linkopts = [
        "-lrt",
    ],
    deps = [
        #"//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic:openssl-native",

        "@sdk//obj:sdk_logger",

        # PD
        "//nic/model_sim:model_client",
    ],
)

cc_binary(
    name = "nic_proxy-v6e2etest_tls-server",
    srcs = [
        "utls/tls-server-v6.cc",
    ],
    linkopts = [
        "-lrt",
    ],
    deps = [
        #"//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic:openssl-native",
        "@sdk//obj:sdk_logger",

        # PD
        "//nic/model_sim:model_client",
    ],
)

cc_binary(
    name = "nic_proxy-e2etest_tcp-server",
    srcs = [
        "utls/tcp-server.cc",
    ],
    linkopts = [
        "-lrt",
    ],
    deps = [
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic:openssl-native",
        "@sdk//obj:sdk_logger",

        # PD
        "//nic/model_sim:model_client",
    ],
)

cc_binary(
    name = "nic_proxy-v6e2etest_tcp-server",
    srcs = [
        "utls/tcp-server-v6.cc",
    ],
    linkopts = [
        "-lrt",
    ],
    deps = [
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic:openssl-native",
        "@sdk//obj:sdk_logger",

        # PD
        "//nic/model_sim:model_client",
    ],
)


cc_binary(
    name = "nic_proxy-e2etest_hntap",
    srcs = [
        "proxy_hntap.cc"
    ],
    linkopts = [
        "-lrt",
    ],
    deps = [
        "//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests:hntap-lib",
        "//nic/e2etests/proxy:common_includes",
        "//nic/e2etests:hntap_common_includes",
        "//nic/e2etests:e2etest-lib",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic:openssl-native",
        "@sdk//obj:sdk_logger",

        # PD
        "//nic/model_sim:model_client",
    ],
)

cc_binary(
    name = "nic_proxy-v6e2etest_hntap",
    srcs = [
        "proxy_hntap-v6.cc"
    ],
    linkopts = [
        "-lrt",
    ],
    deps = [
        "//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests:hntapv6-lib",
        "//nic/e2etests/proxy:common_includes",
        "//nic/e2etests:hntap_common_includes",
        "//nic/e2etests:e2etest-lib",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic:openssl-native",
        "@sdk//obj:sdk_logger",

        # PD
        "//nic/model_sim:model_client",
    ],
)
