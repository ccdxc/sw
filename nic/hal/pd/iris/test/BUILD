package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_test(
    name = "rw_tbl_test",
    srcs = glob(["rw_tbl_lib/*.cc"]),
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lprotobuf",
    ],
    deps = [
        "//:gtest",
        "//nic/utils/trace",
        "//nic/fte:fte",
        "//nic/hal/plugins/network:network",
        "//nic/hal/plugins/classic:classic",
        "//nic/hal/plugins/proxy:proxyplugin",
        "//nic/utils/host_mem:host_mem",
        "//nic/hal/test/utils:haltestutils",
        "//nic/hal/lkl:lkl_api",
        "//nic:lkl",
        "//nic:halproto",
        "//nic:grpc",

        # PD
        "//nic/hal/pd/common:pdcommon",
        "//nic/hal/pd/iris/p4pd:p4pdapi",
        "//nic/hal/pd/iris",
        "//nic:capricsr_int",
        "//nic/hal/pd/capri",
        "//nic/hal/pd/control",
        "//nic/model_sim:model_client",
        "//nic:p4pd",
        "//nic:p4pluspd_txdma",
        "//nic:p4pluspd_rxdma",
        "//nic:asic_libs",
		"//nic/hal/pd/utils/p4pd_mock:p4pd_mock",
    ],
)

cc_test(
    name = "tnnl_rw_tbl_test",
    srcs = glob(["tnnl_rw_tbl_lib/*.cc"]),
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lprotobuf",
    ],
    deps = [
        "//:gtest",
        "//nic/utils/trace",
        "//nic/fte:fte",
        "//nic/hal/plugins/network:network",
        "//nic/hal/plugins/classic:classic",
        "//nic/hal/plugins/proxy:proxyplugin",
        "//nic/utils/host_mem:host_mem",
        "//nic/hal/test/utils:haltestutils",
        "//nic/hal/lkl:lkl_api",
        "//nic:lkl",

        # PD
        "//nic/hal/pd/common:pdcommon",
        "//nic/hal/pd/iris/p4pd:p4pdapi",
        "//nic/hal/pd/iris",
        "//nic:capricsr_int",
        "//nic/hal/pd/capri",
        "//nic/hal/pd/control",
        "//nic/model_sim:model_client",
        "//nic:p4pd",
        "//nic:p4pluspd_txdma",
        "//nic:p4pluspd_rxdma",
        "//nic:asic_libs",
		"//nic/hal/pd/utils/p4pd_mock:p4pd_mock",
    ],
)
