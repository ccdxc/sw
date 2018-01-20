package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "capricsr_int",
    srcs = [
        "asic/capri/model/cap_top/cap_top_csr.cc",
        "asic/capri/model/cap_top/cap_top_csr.h",
        #"asic/capri/model/cap_top/cap_env_base.cc",
        #"asic/capri/model/cap_top/cap_env_base.h",
        #"asic/capri/model/cap_top/cap_model_base.h",
        #"asic/capri/model/cap_top/cap_factory.h",
        #"asic/capri/model/utils/cap_blk_env_base.h",
        "asic/capri/model/utils/cap_csr_base.cc",
        "asic/capri/model/utils/cap_csr_base.h",
        "asic/capri/model/utils/cpp_int_helper.h",
        "asic/capri/model/utils/cap_blk_reg_model.h",
        "asic/capri/model/cap_prd/cap_pr_csr.cc",
        "asic/capri/model/cap_prd/cap_pr_csr.h",
        "asic/capri/model/cap_ptd/cap_pt_csr.cc",
        "asic/capri/model/cap_ptd/cap_pt_csr.h",
        "asic/capri/model/cap_psp/cap_psp_csr.cc",
        "asic/capri/model/cap_psp/cap_psp_csr.h",
        "asic/capri/model/cap_ppa/cap_ppa_csr.cc",
        "asic/capri/model/cap_ppa/cap_ppa_csr.h",
        "asic/capri/model/cap_prd/cap_prd_csr.cc",
        "asic/capri/model/cap_prd/cap_prd_csr.h",
        "asic/capri/model/cap_ptd/cap_ptd_csr.cc",
        "asic/capri/model/cap_ptd/cap_ptd_csr.h",
        "asic/capri/model/cap_te/cap_te_csr.cc",
        "asic/capri/model/cap_te/cap_te_csr.h",
        "asic/capri/model/cap_mpu/cap_mpu_csr.cc",
        "asic/capri/model/cap_mpu/cap_mpu_csr.h",
        "asic/capri/model/cap_pic/cap_pics_csr.cc",
        "asic/capri/model/cap_pic/cap_pics_csr.h",
        "asic/capri/model/cap_pcie/cap_pxb_csr.cc",
        "asic/capri/model/cap_pcie/cap_pxb_csr.h",
        "asic/capri/model/cap_pcie/cap_pp_csr.cc",
        "asic/capri/model/cap_pcie/cap_pp_csr.h",
        "asic/capri/model/cap_npv/cap_npv_csr.cc",
        "asic/capri/model/cap_npv/cap_npv_csr.h",
        "asic/capri/model/cap_pic/cap_pict_csr.cc",
        "asic/capri/model/cap_pic/cap_pict_csr.h",
        "asic/capri/model/cap_txs/cap_txs_csr.cc",
        "asic/capri/model/cap_txs/cap_txs_csr.h",
        "asic/capri/model/cap_pb/cap_pbc_csr.cc",
        "asic/capri/model/cap_pb/cap_pbc_csr.h",
        "asic/capri/model/cap_pb/cap_pbm_csr.cc",
        "asic/capri/model/cap_pb/cap_pbm_csr.h",
        "asic/capri/model/cap_pb/cap_pbchbmtx_csr.cc",
        "asic/capri/model/cap_pb/cap_pbchbmtx_csr.h",
        "asic/capri/model/cap_pb/cap_pbchbmeth_csr.cc",
        "asic/capri/model/cap_pb/cap_pbchbmeth_csr.h",
        "asic/capri/model/cap_pb/cap_pbchbm_csr.cc",
        "asic/capri/model/cap_pb/cap_pbchbm_csr.h",
        "asic/capri/model/cap_pb/cap_pbc_decoders.h",
        "asic/capri/model/cap_pb/cap_pbc_decoders.cc",
        "asic/capri/model/cap_wa/cap_wa_csr.cc",
        "asic/capri/model/cap_wa/cap_wa_csr.h",
        "asic/capri/model/cap_ms/cap_ms_csr.cc",
        "asic/capri/model/cap_ms/cap_ms_csr.h",
        "asic/capri/model/cap_ms/cap_msr_csr.cc",
        "asic/capri/model/cap_ms/cap_msr_csr.h",
        "asic/capri/model/cap_ms/cap_msh_csr.cc",
        "asic/capri/model/cap_ms/cap_msh_csr.h",
        "asic/capri/model/cap_ms/cap_mss_csr.cc",
        "asic/capri/model/cap_ms/cap_mss_csr.h",
        "asic/capri/model/cap_em/cap_em_csr.cc",
        "asic/capri/model/cap_em/cap_em_csr.h",
        "asic/capri/model/cap_ap/cap_ap_csr.cc",
        "asic/capri/model/cap_ap/cap_ap_csr.h",
        "asic/capri/model/cap_ap/cap_apb_csr.h",
        "asic/capri/model/cap_ap/cap_apb_csr.cc",
        "asic/capri/model/cap_he/readonly/cap_hens_csr.cc",
        "asic/capri/model/cap_he/readonly/cap_hens_csr.h",
        "asic/capri/model/cap_he/readonly/cap_hese_csr.cc",
        "asic/capri/model/cap_he/readonly/cap_hese_csr.h",
        "asic/capri/model/cap_he/readonly/cap_mpns_csr.cc",
        "asic/capri/model/cap_he/readonly/cap_mpns_csr.h",
        "asic/capri/model/cap_he/readonly/cap_mpse_csr.cc",
        "asic/capri/model/cap_he/readonly/cap_mpse_csr.h",
        # "asic/capri/model/cap_he/cap_he_csr.h",
        "asic/capri/model/cap_pcie/cap_pxp_csr.cc",
        "asic/capri/model/cap_pcie/cap_pxp_csr.h",
        "asic/capri/model/cap_pcie/cap_pxc_csr.cc",
        "asic/capri/model/cap_pcie/cap_pxc_csr.h",
        "asic/capri/model/cap_sema/cap_sema_csr.cc",
        "asic/capri/model/cap_sema/cap_sema_csr.h",
        "asic/capri/model/cap_intr/cap_intr_csr.cc",
        "asic/capri/model/cap_intr/cap_intr_csr.h",
        "asic/capri/model/cap_bx/cap_bx_csr.cc",
        "asic/capri/model/cap_bx/cap_bx_csr.h",
        "asic/capri/model/cap_mx/cap_mx_csr.cc",
        "asic/capri/model/cap_mx/cap_mx_csr.h",
        "asic/capri/model/cap_mc/cap_mc_csr.cc",
        "asic/capri/model/cap_mc/cap_mc_csr.h",
        "asic/capri/model/cap_mc/cap_mch_csr.cc",
        "asic/capri/model/cap_mc/cap_mch_csr.h",
        "asic/capri/model/utils/LogMsg.cc",
        "asic/capri/model/utils/LogMsg.h",
        "asic/capri/model/utils/common_dpi.h",
        "asic/capri/model/utils/msg_man.h",
        "asic/capri/model/utils/msg_man.cc",
        "asic/capri/model/utils/msg_stream.h",
        "asic/capri/model/utils/msg_stream.cc",
        "asic/capri/model/utils/cap_csr_py_if.cc",
        "asic/capri/model/utils/cap_csr_py_if.h",
        #"asic/capri/model/utils/cap_blk_env_top.cc",
        #"asic/capri/model/utils/cap_test_factory.h",
        #"asic/capri/model/utils/cap_blk_env_top.h",
        "asic/ip/verif/pcpp/pen_axi4_slave.h",
        "asic/ip/verif/pcpp/HBM.h",
        "asic/ip/verif/pcpp/HOST_MEM.h",
        "asic/ip/verif/pcpp/cpu.cc",
        "asic/ip/verif/pcpp/cpu.h",
        "asic/ip/verif/pcpp/cpu_bus_base.h",
        "asic/ip/verif/pcpp/cpu_bus_stub.h",
        "asic/ip/verif/pcpp/pen_mem.h",
        "asic/ip/verif/pcpp/pknobs.h",
        "asic/ip/verif/pcpp/pknobs.cc", 
        "asic/ip/verif/pcpp/pen_mem_base.h",
        "asic/ip/verif/pcpp/pknobs_holder.h",
        "asic/capri/verif/apis/cap_pb_api.cc",
        "asic/capri/verif/apis/cap_txs_api.cc",
        "asic/capri/design/common/gen/cap_phv_intr_decoders.cc",
        "asic/capri/design/common/gen/cap_phv_intr_decoders.h",
        "asic/capri/design/common/gen/cap_axi_decoders.h",
        #"asic/capri/design/common/gen/cap_axi_decoders.h",
    ]  + glob(["asic/capri/model/cap_dpa/*csr.cc", "asic/capri/model/cap_dpa/*csr.h"])
       + glob(["asic/capri/verif/apis/*.h"])
       + glob(["asic/capri/model/cap_pb/*port*csr.cc", "asic/capri/model/cap_pb/*port*csr.h"]),
    includes = [
        "asic/capri/model/cap_top",
        "asic/capri/model/utils",
        "asic/capri/model/cap_prd",
        "asic/capri/model/cap_ptd",
        "asic/capri/model/cap_psp",
        "asic/capri/model/cap_ppa",
        "asic/capri/model/cap_prd",
        "asic/capri/model/cap_ptd",
        "asic/capri/model/cap_te",
        "asic/capri/model/cap_mpu",
        "asic/capri/model/cap_pic",
        "asic/capri/model/cap_pcie",
        "asic/capri/model/cap_npv",
        "asic/capri/model/cap_pic",
        "asic/capri/model/cap_txs",
        "asic/capri/model/cap_pb",
        "asic/capri/model/cap_wa",
        "asic/capri/model/cap_ms",
        "asic/capri/model/cap_em",
        "asic/capri/model/cap_mc",
        "asic/capri/model/cap_he",
        "asic/capri/model/cap_dpa",
        "asic/capri/model/cap_sema",
        "asic/capri/model/cap_intr",
        "asic/capri/model/cap_mx",
        "asic/capri/model/cap_ap",
        "asic/capri/model/cap_bx",
        "asic/capri/model/cap_he/readonly",
        "asic/ip/verif/pcpp",
        "asic/capri/design/common/gen",
    ],
    deps = ["//nic/hal/pd/capri/csr:sknobs_hdrs"],
    linkopts = [
            "-lJudy",
            ],
    copts = [
            "-DEXCLUDE_PER_FIELD_CNTRL",
            "-DCAPRI_HAL",
            "-DCAP_CSR_LARGE_ARRAY_THRESHOLD=1024",
            # Disabling warnings
            "-Wno-unused-function",
            "-Wno-unused-variable",
            "-Wno-sign-compare",
            "-Wno-maybe-uninitialized",
            "-Wno-unused-but-set-variable",
            ],
    hdrs =  [
            "asic/capri/design/common/gen/cap_pb_port_def.gh",
            ]
)

cc_library(
    name = "asic_libs",
    srcs = select ({
        "//conditions:default": [
            "asic/capri/model/capsim-master/gen/x86_64/lib/libmpuobj.so",
            "asic/capri/model/capsim-master/gen/x86_64/lib/libcapisa.so",
            "asic/capri/model/capsim-master/gen/x86_64/lib/libisa.so",
        ],

        ":aarch64_mode" : [
            "asic/capri/model/capsim-master/gen/aarch64/lib/libmpuobj.so",
            "asic/capri/model/capsim-master/gen/aarch64/lib/libcapisa.so",
            "asic/capri/model/capsim-master/gen/aarch64/lib/libisa.so",
        ],
    })
)

cc_library(
    name = "asic_includes",
    srcs = [],
    hdrs = [
        "asic/capri/model/capsim-master/lib/libcapisa/include/libcapisa.h",
    ] + glob([
        "asic/capri/model/capsim-master/lib/libmpuobj/include/*.h",
    ]),
    includes = [
        "asic/capri/model/capsim-master/lib/libmpuobj/include",
    ],
    deps = [],
)

cc_library(
    name = "p4pd",
    srcs = ["gen/iris/src/p4pd.cc"],
    hdrs = [
        "gen/iris/include/p4pd.h",
    ],
    deps = [
        "//nic:gen_includes",
        "//nic/include:base_includes",
    ],
)

cc_library(
    name = "gft_p4pd",
    srcs = ["gen/gft/src/gft_p4pd.cc"],
    hdrs = [
        "gen/gft/include/gft_p4pd.h",
    ],
    deps = [
        "//nic:gen_includes",
        "//nic/include:base_includes",
    ],
)

cc_library(
    name = "p4pluspd_txdma",
    srcs = ["gen/common_txdma_actions/src/common_txdma_actions_p4pd.cc"],
    hdrs = [
        "gen/common_txdma_actions/include/common_txdma_actions_p4pd.h",
    ],
    deps = [
        "//nic:gen_includes",
        "//nic/include:base_includes",
    ],
)

cc_library(
    name = "p4pluspd_rxdma",
    srcs = ["gen/common_rxdma_actions/src/common_rxdma_actions_p4pd.cc"],
    hdrs = [
        "gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h",
    ],
    deps = [
        "//nic:gen_includes",
        "//nic/include:base_includes",
    ],
)

cc_library(
    name = "gen_includes",
    srcs = [],
    hdrs = ["gen/iris/include/p4pd.h",
            "gen/iris/include/p4plus_egress.h",
            "gen/iris/include/p4plus_ingress.h",
            "gen/gft/include/gft_p4pd.h",
            "gen/tcp_proxy_rxdma/include/tcp_proxy_rxdma_p4plus_ingress.h",
            "gen/tcp_proxy_txdma/include/tcp_proxy_txdma_p4plus_ingress.h",
            "gen/common_txdma_actions/include/common_txdma_actions_p4pd.h",
            "gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h",
            #"gen/ipsec_rxdma_actions/include/ipsec_rxdma_actions_p4plus_ingress.h",
            "gen/tls_txdma_pre_crypto_enc/include/tls_txdma_pre_crypto_enc_p4plus_ingress.h",
            "gen/tls_txdma_post_crypto_enc/include/tls_txdma_post_crypto_enc_p4plus_ingress.h",
            "gen/cpu_rxdma/include/cpu_rxdma_p4plus_ingress.h",
            "gen/cpu_txdma/include/cpu_txdma_p4plus_ingress.h",
            "gen/gc_txdma/include/gc_txdma_p4plus_ingress.h",
            "gen/rawr_rxdma/include/rawr_rxdma_p4plus_ingress.h",
            "gen/rawc_txdma/include/rawc_txdma_p4plus_ingress.h",
            "gen/proxyr_txdma/include/proxyr_txdma_p4plus_ingress.h",
            "gen/proxyc_txdma/include/proxyc_txdma_p4plus_ingress.h",
            "gen/esp_ipv4_tunnel_h2n_rxdma/include/esp_ipv4_tunnel_h2n_rxdma_p4plus_ingress.h",
            "gen/esp_v4_tunnel_n2h_rxdma/include/esp_v4_tunnel_n2h_rxdma_p4plus_ingress.h",
            ],
    deps = ["//nic/include:base_includes"],
)

cc_library(
    name = "gen_proto_includes",
    srcs = [
    ] + glob(["gen/proto/hal/*.hpp"]),
    hdrs = glob([
        "gen/proto/hal/*.hpp",
        "gen/proto/hal/*.h",
    ]),
)

cc_library(
    name = "halproto",
    srcs = [
        "obj/libhalproto.so",
    ],
    linkopts = [
        "-Lnic/obj",
    ],
)

cc_library(
    name = "grpc",

    srcs = select({
        "//conditions:default": [
            "third-party/grpc/x86_64/libgrpc++_reflection.a",
            "third-party/grpc/x86_64/libgrpc++.a",
            "third-party/grpc/x86_64/libgrpc_unsecure.a",
            "third-party/grpc/x86_64/libgrpc++_unsecure.a",
        ],

        ":aarch64_mode" : [
            "third-party/grpc/aarch64/libgrpc++_reflection.a",
            "third-party/grpc/aarch64/libgrpc++.a",
            "third-party/grpc/aarch64/libgrpc_unsecure.a",
            "third-party/grpc/aarch64/libgrpc++_unsecure.a",
        ],
    }),

    linkopts = select ({
        "//conditions:default" : [
            "-Lthird-party/grpc/x86_64",
        ],

        ":aarch64_mode" : [
            "-Lthird-party/grpc/aarch64",
        ],
    })
)

cc_library(
    name = "lkl_includes",

    srcs = [
        "third-party/lkl/export/include/lkl.h",
        "third-party/lkl/export/include/lkl_host.h",
    ],
    hdrs = [
        "third-party/lkl/export/include/lkl.h",
        "third-party/lkl/export/include/lkl_host.h",
        "third-party/lkl/export/include/lkl/autoconf.h",
        #"third-party/lkl/export/include/lkl/asm/syscalls.h",
        #"third-party/lkl/export/include/lkl/linux/types.h",
        #"third-party/lkl/export/include/lkl/linux/stddef.h",
        #"third-party/lkl/export/include/lkl/linux/posix_types.h",
        #"third-party/lkl/export/include/lkl/asm/types.h",
        #"third-party/lkl/export/include/lkl/asm/posix_types.h",
        #"third-party/lkl/export/include/lkl/asm/bitsperlong.h",
        #"third-party/lkl/export/include/lkl/asm-generic/types.h",
        #"third-party/lkl/export/include/lkl/asm-generic/posix_types.h",
        #"third-party/lkl/export/include/lkl/asm-generic/int-ll64.h",
    ] + glob(["third-party/lkl/export/include/lkl/linux/*.h",
              "third-party/lkl/export/include/lkl/asm/*.h",
              "third-party/lkl/export/include/lkl/asm-generic/*.h",
              "third-party/lkl/export/include/lkl/linux/hdlc/*.h",
              "third-party/lkl/export/include/lkl/linux/byteorder/*.h"]),

    includes = [
        "third-party/lkl/export/include",
    ],

    deps = [],
)

cc_library(
    name = "lkl",

    srcs = select ({
        "//conditions:default" : [
            "third-party/lkl/export/x86_64/bin/liblkl.so",
        ],

        ":aarch64_mode" : [
            "third-party/lkl/export/aarch64/bin/liblkl.so",
        ],
    }),

    linkopts = select ({
        "//conditions:default" : [
            "-Lthird-party/lkl/export/x86_64/bin"
        ],

        ":aarch64_mode" : [
            "-Lthird-party/lkl/export/aarch64/bin"
        ],
    }),
)

cc_library(
    name = "openssl",

    hdrs = glob(["third-party/openssl/include/openssl/*.h"]),

    srcs = select ({
        "//conditions:default" : [
            "third-party/openssl/x86_64/lib/libssl.a",
            "third-party/openssl/x86_64/lib/libcrypto.a",
        ],
        ":aarch64_mode" : [
            "third-party/openssl/aarch64/lib/libssl.a",
            "third-party/openssl/aarch64/lib/libcrypto.a",
        ],
    }),

    includes = [
        "third-party/openssl/include",
    ],

    linkopts = select ({
        "//conditions:default" : [
            "-Lthird-party/openssl/x86_64/lib -ldl",
        ],
        ":aarch64_mode" : [
            "-Lthird-party/openssl/aarch64/lib -ldl",
        ],
    }),
)

cc_library(
    name = "openssl-native",
    hdrs = glob(["third-party/openssl-native/include/openssl/*.h"]),

    srcs = [
        "third-party/openssl-native/lib/libssl.a",
        "third-party/openssl-native/lib/libcrypto.a",
    ],

    includes = [
        "third-party/openssl-native/include",
    ],

    linkopts = [
        "-Lthird-party/openssl-native/lib -ldl",
    ],
)

cc_library(
    name = "snort3",
    srcs = [
        "third-party/snort3/export/bin/libsnort.so",
    ],
    hdrs = glob(["third-party/snort3/export/include/*.h"]),
    includes = [
        "third-party/snort3/export/include",
    ],
)

cc_library(
    name = "libtirpc",
    hdrs = glob(["third-party/libtirpc/export/*.h"]),

    includes = [
        "third-party/libtirpc/export"
    ],
)

config_setting(
    name = "aarch64_mode",
    values = { "cpu": "aarch64-linux" }
)

# Used by dol/test/storage/BUILD
cc_inc_library(
    name = "compression_asic_hdrs",
    hdrs = [
        "asic/capri/design/common/cap_addr_define.h",
        "asic/capri/model/cap_he/readonly/cap_hens_csr_define.h",
    ],
)

exports_files(["tools/ncc/tenjin_wrapper.py", 
               "asic/capri/model/cap_pb/cap_pbc_csr.json"])
