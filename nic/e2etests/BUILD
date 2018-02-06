package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "driver",
    srcs = [
             "driver/lib_driver.cc",
           ],
    hdrs = glob(["driver/*.hpp"]),
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
    name = "nic_proxy-e2etest_hntap",
    srcs = [
        "hntap/hntap.cc"
    ],
    deps = [
        "//nic/model_sim/include:modelsim_includes",
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

