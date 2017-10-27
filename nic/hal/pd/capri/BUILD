package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "capri_includes",
    srcs = [],
    hdrs = [
        "capri_hbm.hpp",
        "capri_loader.h",
        "capri_barco_rings.hpp",
        "capri_tbl_rw.hpp",
    ],
)

cc_library(
    name = "capri",
    srcs = [
            "capri_tbl_rw.cc",
            "capri_loader.cc",
            "capri_qstate.cc",
            "capri_config.cc",
            "capri_hbm.cc",
            "capri_asic_rw.cc",
            "capri_barco_crypto.cc",
            "capri_tm_rw.cc",
            "capri_init.cc",
            "capri_repl.cc",
            "capri_txs_scheduler.cc",
		    "capri_barco_rings.cc",
            "capri_barco_res.cc",
            "capri_barco_asym_apis.cc",
            "capri_barco_asym_tests.cc"
            ],
    hdrs = [
    ] + glob(["*.hpp"]) + glob(["*.h"]),
    deps = [
        "//nic:asic_includes",
        "//nic:capricsr_int",
        "//nic/hal:hal_src",
        "//nic/hal/pd/capri/csr:capricsr",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/p4/nw/include:p4_nw_includes",
        "//nic/third-party/spdlog",
    ],
    alwayslink = 1,
)
