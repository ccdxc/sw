package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "capri_includes",
    srcs = [],
    hdrs = [
        "capri_hbm.hpp",
        "capri_loader.h",
        "capri_barco_rings.hpp",
        "capri_tbl_rw.hpp",
        "capri_barco_asym_apis.hpp",
        "capri_barco_sym_apis.hpp",
        "capri_barco_crypto.hpp",
        "capri_pxb_pcie.hpp",
        "capri_tm_rw.hpp",
    ],
)

capri_util_deps = [
    "//nic/utils/bm_allocator:bm_allocator",
    "@sdk//obj:sdk_indexer",
    "@sdk//obj:sdk_slab",
]

capri_pd_deps = [
    "//nic/hal/pd/capri/csr:capricsr",
    "//nic/hal/pd/capri/csr:hal_pknobs",
    "//nic/hal/pd:pdcommon",
    "//nic/hal/pd/iris:iris_includes",
]

cc_library(
    name = "capri",
    srcs = [
        "capri_barco_asym_apis.cc",
        "capri_barco_asym_tests.cc",
        "capri_barco_crypto.cc",
        "capri_barco_res.cc",
        "capri_barco_rings.cc",
        "capri_barco_sym_apis.cc",
        "capri_barco_sym_tests.cc",
        "capri_config.cc",
        "capri_hbm.cc",
        "capri_init.cc",
        "capri_loader.cc",
        "capri_pxb_pcie.cc",
        "capri_qstate.cc",
        "capri_repl.cc",
        "capri_tbl_rw.cc",
        "capri_tm_rw.cc",
        "capri_txs_scheduler.cc",
        "capri_pd.cc",
    ],
    hdrs = [
    ] + glob(["*.hpp"]) + glob(["*.h"]),
    copts = [
        "-DCAPRI_HAL",
        "-DCAP_CSR_LARGE_ARRAY_THRESHOLD=1024",
    ] + sdk_copts,
    deps = [
               "//nic:asic_includes",
               "//nic:gen_includes",
               "//nic:gen_proto_includes",
               "//nic:capricsr_int",
               "//nic/include:base_includes",
               "//nic/hal/lib:hal_lib_includes",
               "//nic/utils/block_list:block_list",
               "//nic/model_sim/include:modelsim_includes",
               "//nic/p4/nw/include:p4_nw_includes",
               "//nic:spdlog",
               "//nic:google_includes",
               "//nic:grpc_includes",
               "//nic/hal/periodic:periodic",
           ] + capri_util_deps +
           capri_pd_deps,
    alwayslink = 1,
)

genrule(
    name = "capri_tm_rw_gen",
    srcs = ["//nic:tools/ncc/tenjin_wrapper.py", ":capri_tm_rw.template.cc",
            "//nic:asic/capri/model/cap_pb/cap_pbc_csr.json"
            ],
    outs = [":capri_tm_rw.cc"],
    cmd = "python2 $(location //nic:tools/ncc/tenjin_wrapper.py) \
           --template $(location :capri_tm_rw.template.cc) \
           --outfile $(location :capri_tm_rw.cc) \
           --args $(location //nic:asic/capri/model/cap_pb/cap_pbc_csr.json)"
)
