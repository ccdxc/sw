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
        "//nic/include:base_includes",
    ]
)

cc_library(
    name= "nicmgr_if",
    srcs = [
        "nicmgr_if.cc",
        "nicmgr_if.hpp",
        "tests.hpp",
        "dp_mem.hpp",
        "chain_params.hpp",
        "utils.hpp",
    ],
    hdrs = [
        "nicmgr_if.hpp",
    ],
    copts = [ 
              "-g",
              "-Inic/include",
              "-Inic/e2etests/driver",
              "-Inic/gen/proto/hal",
              "-Iplatform/src/lib/nicmgr/include",
              "-Iplatform/include/common",
              "-Iplatform/src/lib/misc/include",
              "-Iplatform/src/lib/cfgspace/include",
              "-Iplatform/src/lib/pciehdevices/include",
              "-Iplatform/src/lib/pcieport/include",
            ],
    deps = [
        "//external:gflags",
        "//nic/include:base_includes",
        "//nic/e2etests:driver",
        "//nic:gen_proto_includes",
        "//platform/src/lib/nicmgr/include:libnicmgr_hdrs",
        "//platform/include/common:platformcommon_hdrs",
        "//platform/src/lib/misc/include:libmisc_hdrs",
        "//platform/src/lib/cfgspace/include:libcfgspace_hdrs",
        "//platform/src/lib/pciehdevices/include:libpciehdevices_hdrs",
        "//platform/src/lib/pcieport/include:libpcieport_hdrs",
        ":hal_if",
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
        "//nic/include:base_includes",
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
        "chain_params.hpp",
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
        "//nic:compression_asic_hdrs",
        "//nic/include:base_includes",
    ]
)

cc_library(
    name = "compression",
    srcs = [
        "compression.cc",
        "compression.hpp",
        "compression_test.hpp",
        "comp_encrypt_chain.hpp",
        "comp_hash_chain.hpp",
        "chksum_decomp_chain.hpp",
        "decrypt_decomp_chain.hpp",
        "utils.hpp",
        "dp_mem.hpp",
	"queues.hpp",
	"xts.hpp",
        "tests.hpp",
        "chain_params.hpp",
        "acc_ring.hpp",
        "storage_seq_p4pd.hpp",
    ],
    hdrs = ["compression.hpp"],
    copts = [ "-g" ],
    deps = [
        "//nic:compression_asic_hdrs",
        "//nic:p4pluspd_storage_seq",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//external:gflags",
        "//nic/include:base_includes",
    ],
)

cc_library(
    name = "acc_scale_tests",
    srcs = [
        "acc_scale_tests.cc",
        "acc_scale_tests.hpp",
        "acc_ring.hpp",
        "compression.hpp",
        "compression_test.hpp",
        "comp_encrypt_chain.hpp",
        "comp_hash_chain.hpp",
        "chksum_decomp_chain.hpp",
        "decrypt_decomp_chain.hpp",
        "encrypt_only.hpp",
        "utils.hpp",
        "dp_mem.hpp",
	"queues.hpp",
	"xts.hpp",
        "tests.hpp",
        "chain_params.hpp",
    ],
    hdrs = ["acc_scale_tests.hpp"],
    copts = [ "-g" ],
    deps = [
        "//nic:compression_asic_hdrs",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//external:gflags",
        "//nic/include:base_includes",
    ],
)

cc_library(
    name = "acc_ring",
    srcs = [
        "acc_ring.cc",
        "acc_ring.hpp",
        "compression.hpp",
        "utils.hpp",
        "dp_mem.hpp",
	"queues.hpp",
        "tests.hpp",
        "chain_params.hpp",
        "storage_seq_p4pd.hpp",
    ],
    hdrs = ["acc_ring.hpp"],
    copts = [ "-g" ],
    deps = [
        "//nic:compression_asic_hdrs",
        "//nic:p4pluspd_storage_seq",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//external:gflags",
        "//nic/include:base_includes",
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
        "chain_params.hpp",
        "acc_ring.hpp",
    ],
    hdrs = ["comp_encrypt_chain.hpp"],
    copts = [ "-g" ],
    deps = [
        "//nic:compression_asic_hdrs",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//external:gflags",
        "//nic/include:base_includes",
    ],
)

cc_library(
    name = "comp_hash_chain",
    srcs = [
        "comp_hash_chain.cc",
        "comp_hash_chain.hpp",
        "compression.hpp",
        "compression_test.hpp",
        "utils.hpp",
        "dp_mem.hpp",
	"queues.hpp",
        "tests.hpp",
        "chain_params.hpp",
        "acc_ring.hpp",
    ],
    hdrs = ["comp_hash_chain.hpp"],
    copts = [ "-g" ],
    deps = [
        "//nic:compression_asic_hdrs",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//external:gflags",
        "//nic/include:base_includes",
    ],
)

cc_library(
    name = "chksum_decomp_chain",
    srcs = [
        "chksum_decomp_chain.cc",
        "chksum_decomp_chain.hpp",
        "comp_hash_chain.hpp",
        "compression.hpp",
        "compression_test.hpp",
        "utils.hpp",
        "dp_mem.hpp",
	"queues.hpp",
	"xts.hpp",
        "tests.hpp",
        "chain_params.hpp",
        "acc_ring.hpp",
    ],
    hdrs = ["chksum_decomp_chain.hpp"],
    copts = [ "-g" ],
    deps = [
        "//nic:compression_asic_hdrs",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//external:gflags",
        "//nic/include:base_includes",
    ],
)

cc_library(
    name = "encrypt_only",
    srcs = [
        "encrypt_only.cc",
        "encrypt_only.hpp",
        "compression.hpp",
        "compression_test.hpp",
        "utils.hpp",
        "dp_mem.hpp",
	"queues.hpp",
	"xts.hpp",
        "tests.hpp",
        "chain_params.hpp",
        "acc_ring.hpp",
    ],
    hdrs = ["encrypt_only.hpp"],
    copts = [ "-g" ],
    deps = [
        "//nic:compression_asic_hdrs",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//external:gflags",
        "//nic/include:base_includes",
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
        "chain_params.hpp",
        "acc_ring.hpp",
    ],
    hdrs = ["decrypt_decomp_chain.hpp"],
    copts = [ "-g" ],
    deps = [
        "//nic:compression_asic_hdrs",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//external:gflags",
        "//nic/include:base_includes",
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
        "chain_params.hpp",
        "compression.hpp",
        "storage_seq_p4pd.hpp",
        "nvme.hpp",
        "xts.hpp",
        "xts.cc",
        "acc_ring.cc",
	"acc_ring.hpp",
        "xts_tests.cc",
        "e2e_perf_tests.cc",
    ],
    deps = [
        ":hal_if",
        ":nicmgr_if",
        ":qstate_if",
        ":r2n",
        ":ssd",
        ":utils",
        "//nic:p4pluspd_storage_seq",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//nic/include:base_includes",
    ],
    copts = [
        "-std=c++14",
        "-g",
        "-Inic/include",
    ]
)

cc_library(
    name= "nvme_dp",
    srcs = [
        "nvme.hpp",
        "nvme_dp.cc",
        "nvme_dp.hpp",
	"acc_ring.hpp",
        "queues.cc",
        "queues.hpp",
        "dp_mem.cc",
        "dp_mem.hpp",
        "compression.hpp",
        "xts.hpp",
        "chain_params.hpp",
        "rdma.hpp",
    ],
    deps = [
        ":hal_if",
        ":nicmgr_if",
        ":qstate_if",
        ":r2n",
        ":ssd",
        ":utils",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/model_sim:model_client",
        "//nic/include:base_includes",
    ],
    copts = [
        "-std=c++14",
        "-g",
        "-Inic/include",
    ]
)

cc_binary(
    name = "storage_test",
    srcs = [
        "main.cc",
    ],
    deps = [
        ":hal_if",
        ":nicmgr_if",
        ":qstate_if",
        ":tests",
        ":rdma",
        ":compression",
        ":acc_ring",
        ":acc_scale_tests",
        ":comp_encrypt_chain",
        ":comp_hash_chain",
        ":chksum_decomp_chain",
        ":decrypt_decomp_chain",
        ":encrypt_only",
        ":nvme_dp",
        "//nic:p4pluspd_storage_seq",
        "//nic:grpc",
        "//nic:libprotobuf",
        "//external:gflags",
        "//nic/hal/pd/p4pd:p4pd_utils",
    ],
    copts = [ "-g" ],
    linkopts = [
        "-lpthread",
        "-pthread",
        "-lz",
        "-lm",
    ],
)
