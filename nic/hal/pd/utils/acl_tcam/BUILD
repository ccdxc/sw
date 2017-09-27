package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "acl_tcam",
    srcs = [
        "acl_tcam.cc",
        "acl_tcam.hpp",
    ],
    hdrs = ["acl_tcam.hpp"],
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        "//nic/hal/pd/utils/tcam:tcam_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/bitmap",
        "//nic/utils/thread",
    ],
)

cc_library(
    name = "acl_tcam_includes",
    srcs = [],
    hdrs = ["acl_tcam.hpp"],
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/bitmap",
        "//nic/utils/thread",
    ],
)

cc_test(
    name = "acl_tcam_test",
    srcs = ["test/acl_tcam_test.cc"],
    deps = [
        ":acl_tcam",
        "//:gtest",
		"//nic/utils/trace",
        "//nic/fte:fte",
        "//nic/hal/plugins/network:network",
        "//nic/hal/plugins/proxy:proxyplugin",
        "//nic/utils/host_mem:host_mem",
        "//nic/hal/test/utils:haltestutils",
        "//nic/hal/lkl:lkl_api",
        "//nic:lkl",

		# PD
		"//nic/hal/pd/common:pdcommon",
		"//nic/hal/pd/utils/directmap:directmap",
		"//nic/hal/pd/utils/hash:hash",
		"//nic/hal/pd/utils/tcam:tcam",
		"//nic/hal/pd/utils/flow:flow",
		"//nic/hal/pd/utils/met:met",
		"//nic/hal/pd/iris/p4pd:p4pdapi",
		"//nic/hal/pd/iris",
		"//nic:capricsr_int",
		"//nic/hal/pd/capri",
		"//nic/model_sim:model_client",
		"//nic:p4pd",
		"//nic:p4pdtcprx",
		"//nic:p4pdtcptx",
		"//nic:p4pluspd_txdma",
		"//nic:p4pluspd_rxdma",
		"//nic:asic_libs",
        "//nic/hal/pd/utils/p4pd_mock:p4pd_mock"
		],
   linkopts = [
         "-lzmq",
     ],
	)
