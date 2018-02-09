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
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
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

cc_library(
    name = "hntap_common_includes",
    hdrs = glob(["hntap/*.hpp"]),
)


cc_library(
    name = "e2etest-lib",
    srcs = [
             "lib/packet.cc",
           ],
    hdrs = glob(["lib/*.hpp"]),
)


cc_library(
    name = "hntap-lib",
    srcs = [
        	"hntap/dev.cc",
        	"hntap/hntap.cc"
           ],
    hdrs = glob(["hntap/*.hpp"]),
        deps = [
        "//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/e2etests:driver",
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

cc_library(
    name = "nic_proxy-e2etest_hntap",
    srcs = [
        "hntap/hntap.cc",
        "hntap/dev.cc",
    ],
    deps = [
        "//nic/model_sim/include:modelsim_includes",
        # PI
        "//nic/e2etests:driver",
        "//nic/e2etests:hntap_common_includes",
	"//nic/e2etests:e2etest-lib",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic:openssl-native",

        # PD
        "//nic/model_sim:model_client",
    ],
)

