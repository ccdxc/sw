package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name= "hal_if",
    srcs = [
        "hal_if.cc",
        "hal_if.hpp",
    ],
    hdrs = [
        "hal_if.hpp",
    ],
    copts = [ "-g" ],
    deps = [
        "//external:gflags",
        "//nic:halproto",
        "//nic:gen_proto_includes",
        "//nic:grpc_includes",
        "//nic:google_includes",
    ]
)

cc_library(
    name= "utils",
    srcs = [
        "utils.cc",
        "utils.hpp",
    ],
    copts = [ "-g" ],
    deps = [
        ":hal_if",
    ]
)

cc_library(
    name= "qstate_if",
    srcs = [
        "qstate_if.cc",
        "qstate_if.hpp",
    ],
    copts = [ "-g" ],
    deps = [
        ":hal_if",
        ":utils",
        "//nic/model_sim:model_client",
    ]
)

cc_library(
    name = "rdma",
    srcs = [
        "rdma.cc",
        "rdma.hpp",
        "utils.hpp",
	"queues.hpp",
        "tests.hpp",
        "dp_mem.hpp",
        "r2n.hpp",
        "nvme.hpp",
        "qstate_if.hpp",
    ],
    copts = [ "-g" ],
    deps = [
        ":hal_if",
        ":qstate_if",
        "//nic:halproto",
        "//nic:google_includes",
        "//nic:gen_proto_includes",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
    ]
)

cc_library(
    name = "compression",
    srcs = [
        "compression.cc",
        "compression.hpp",
        "compression_test.hpp",
        "comp_encrypt_chain.hpp",
        "decrypt_decomp_chain.hpp",
        "utils.hpp",
        "dp_mem.hpp",
	"queues.hpp",
	"xts.hpp",
        "tests.hpp",
    ],
    hdrs = ["compression.hpp"],
    copts = [ "-g" ],
    deps = [
        "//nic:compression_asic_hdrs",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//external:gflags",
    ],
)

cc_library(
    name = "acc_scale_tests",
    srcs = [
        "acc_scale_tests.cc",
        "acc_scale_tests.hpp",
        "compression.hpp",
        "compression_test.hpp",
        "comp_encrypt_chain.hpp",
        "decrypt_decomp_chain.hpp",
        "utils.hpp",
        "dp_mem.hpp",
	"queues.hpp",
	"xts.hpp",
        "tests.hpp",
    ],
    hdrs = ["acc_scale_tests.hpp"],
    copts = [ "-g" ],
    deps = [
        "//nic:compression_asic_hdrs",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//external:gflags",
    ],
)

cc_library(
    name = "comp_encrypt_chain",
    srcs = [
        "comp_encrypt_chain.cc",
        "comp_encrypt_chain.hpp",
        "compression.hpp",
        "compression_test.hpp",
        "utils.hpp",
        "dp_mem.hpp",
	"queues.hpp",
	"xts.hpp",
        "tests.hpp",
    ],
    hdrs = ["comp_encrypt_chain.hpp"],
    copts = [ "-g" ],
    deps = [
        "//nic:compression_asic_hdrs",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//external:gflags",
    ],
)

cc_library(
    name = "decrypt_decomp_chain",
    srcs = [
        "decrypt_decomp_chain.cc",
        "comp_encrypt_chain.hpp",
        "decrypt_decomp_chain.hpp",
        "compression.hpp",
        "compression_test.hpp",
        "utils.hpp",
        "dp_mem.hpp",
	"queues.hpp",
	"xts.hpp",
        "tests.hpp",
    ],
    hdrs = ["decrypt_decomp_chain.hpp"],
    copts = [ "-g" ],
    deps = [
        "//nic:compression_asic_hdrs",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//external:gflags",
    ],
)

cc_library(
    name= "r2n",
    srcs = [
        "r2n.cc",
        "r2n.hpp",
        "nvme.hpp",
        "dp_mem.hpp",
    ],
    copts = [ "-g" ],
    deps = [
        ":utils",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
    ]
)

cc_library(
    name = "ssd_core",
    srcs = [
        "ssd_core.cc",
        "ssd_core.hpp",
        "nvme.hpp",
        "dp_mem.hpp",
    ],
    hdrs = [
        "ssd_core.hpp",
    ],
    copts = [ "-g" ],
    deps = [
        ":utils",
    ]
)

cc_test(
    name = "ssd_core_test",
    srcs = [
        "ssd_core_test.cc",
    ],
    copts = [ "-g" ],
    deps = [
        ":ssd_core",
        "//:gtest",
    ],
)

cc_library(
    name = "ssd",
    srcs = [
        "ssd.hpp",
    ],
    hdrs = [
        "ssd.hpp",
    ],
    copts = [ "-g" ],
    deps = [
        ":ssd_core",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
    ]
)

cc_library(
    name= "tests",
    srcs = [
        "tests.cc",
	"tests.hpp",
        "rdma.hpp",
        "nvme_dp.cc",
        "nvme_dp.hpp",
        "nvme_dp_tests.cc",
        "nvme_dp_tests.hpp",
        "queues.cc",
        "queues.hpp",
        "dp_mem.cc",
        "dp_mem.hpp",
        "nvme.hpp",
        "xts.hpp",
        "xts.cc",
        "xts_tests.cc",
        "e2e_perf_tests.cc",
    ],
    deps = [
        ":hal_if",
        ":qstate_if",
        ":r2n",
        ":ssd",
        ":utils",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
    ],
    copts = [
        "-std=c++14",
        "-g",
    ]
)

cc_library(
    name= "nvme_dp",
    srcs = [
        "nvme.hpp",
        "nvme_dp.cc",
        "nvme_dp.hpp",
        "queues.cc",
        "queues.hpp",
        "dp_mem.cc",
        "dp_mem.hpp",
        "rdma.hpp",
    ],
    deps = [
        ":hal_if",
        ":qstate_if",
        ":r2n",
        ":ssd",
        ":utils",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
    ],
    copts = [
        "-std=c++14",
        "-g",
    ]
)

cc_binary(
    name = "storage_test",
    srcs = [
        "main.cc",
    ],
    deps = [
        ":hal_if",
        ":qstate_if",
        ":tests",
        ":rdma",
        ":compression",
        ":acc_scale_tests",
        ":comp_encrypt_chain",
        ":decrypt_decomp_chain",
        ":nvme_dp",
        "//nic:grpc",
        "//nic:libprotobuf",
        "//external:gflags",
    ],
    copts = [ "-g" ],
    linkopts = [
        "-lpthread",
        "-pthread",
        "-lz",
        "-lm",
    ],
)
