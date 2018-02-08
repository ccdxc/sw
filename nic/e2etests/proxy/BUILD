package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "common_includes",
    hdrs = glob(["*.hpp"]),
)

cc_binary(
    name = "nic_proxy-e2etest_ethqstate",
    srcs = [
        "test/eth_qstate.cc",
    ],
    deps = [
        "//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/host_mem:host_mem",

        # PD
        "//nic:capricsr_int",
        "//nic/model_sim:model_client",
        "//nic:spdlog",
    ],
)

cc_binary(
    name = "nic_proxy-e2etest_ethtx",
    srcs = [
        "test/eth_tx.cc",
    ],
    deps = [
        "//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem",

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
    deps = [
        "//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem",

        # PD
        "//nic/model_sim:model_client",
    ],
)


cc_binary(
    name = "nic_proxy-e2etest_tls-client",
    srcs = [
        "utls/tls-client.cc",
    ],
    deps = [
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem",
        "//nic:openssl-native",

        # PD
        "//nic/model_sim:model_client",
    ],
)
cc_binary(
    name = "nic_proxy-e2etest_tcp-client",
    srcs = [
        "utls/tcp-client.cc",
    ],
    deps = [
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem",

        # PD
        "//nic/model_sim:model_client",
    ],
)

cc_binary(
    name = "nic_proxy-e2etest_tls-server",
    srcs = [
        "utls/tls-server.cc",
    ],
    deps = [
        #"//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem",
        "//nic:openssl-native",

        # PD
        "//nic/model_sim:model_client",
    ],
)

cc_binary(
    name = "nic_proxy-e2etest_tcp-server",
    srcs = [
        "utls/tcp-server.cc",
    ],
    deps = [
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests/proxy:common_includes",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem",
        "//nic:openssl-native",

        # PD
        "//nic/model_sim:model_client",
    ],
)


cc_binary(
    name = "nic_proxy-e2etest_hntap",
    srcs = [
        "proxy_hntap.cc"
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
        "//nic/utils/host_mem:host_mem",
        "//nic:openssl-native",

        # PD
        "//nic/model_sim:model_client",
    ],
)

