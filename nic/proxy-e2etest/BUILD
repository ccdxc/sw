package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "lib_driver",
    srcs = [
             "lib_driver.cc",
           ],
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/model_sim/include:modelsim_includes",
        "//nic/include:base_includes",
        "//nic/utils/host_mem:host_mem",
        "//nic:gen_includes",
        "//nic:gen_proto_includes",
        "//nic/p4/nw/include:p4_nw_includes",
        "//nic:spdlog",
        "//nic:grpc",
        "//nic:grpc_includes",
        "//nic:google_includes",
        "//nic:libprotobuf",
        "//nic:halproto",
    ],
    linkopts = [
        "-lpthread",
        "-pthread",
        "-Lthird-party/grpc",
        "-lzmq",
        "-lz",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "nic_proxy-e2etest_ethqstate",
    srcs = [
        "test/eth_qstate.cc",
    ],
    deps = [
        "//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/proxy-e2etest:lib_driver",
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
        "//nic/proxy-e2etest:lib_driver",
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
        "//nic/proxy-e2etest:lib_driver",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem",

        # PD
        "//nic/model_sim:model_client",
    ],
)

cc_binary(
    name = "nic_proxy-e2etest_hntap",
    srcs = [
        "test/hntap.cc"
        #"utls/tcp-server.cc"
    ],
    deps = [
        "//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/proxy-e2etest:lib_driver",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem",
        "//nic:openssl-native",

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
        "//nic/proxy-e2etest:lib_driver",
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
        "//nic/proxy-e2etest:lib_driver",
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
        "//nic/proxy-e2etest:lib_driver",
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
        "//nic/proxy-e2etest:lib_driver",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem",
        "//nic:openssl-native",

        # PD
        "//nic/model_sim:model_client",
    ],
)
