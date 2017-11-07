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
    deps = [
        "//nic:halproto",
        "//nic:gen_proto_includes",
    ]
)

cc_library(
    name= "utils",
    srcs = [
        "utils.cc",
        "utils.hpp",
    ],
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
    deps = [
        ":hal_if",
        ":utils",
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
        "r2n.hpp",
        "nvme.hpp",
        "qstate_if.hpp",
    ],
    deps = [
        ":hal_if",
        ":qstate_if",
        "//nic:halproto",
        "//nic:gen_proto_includes",
        "//nic/utils/host_mem",
    ]
)

cc_library(
    name= "r2n",
    srcs = [
        "r2n.cc",
        "r2n.hpp",
        "nvme.hpp",
    ],
    deps = [
        ":utils",
        "//nic/utils/host_mem",
    ]
)

cc_library(
    name = "ssd_core",
    srcs = [
        "ssd_core.cc",
        "ssd_core.hpp",
        "nvme.hpp",
    ],
    hdrs = [
        "ssd_core.hpp",
    ],
    deps = [
        ":utils",
    ]
)

cc_test(
    name = "ssd_core_test",
    srcs = [
        "ssd_core_test.cc",
    ],
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
    deps = [
        ":ssd_core",
        "//nic/utils/host_mem",
        "//nic/model_sim:model_client",
    ]
)

cc_library(
    name= "tests",
    srcs = [
        "tests.cc",
        "tests.hpp",
        "queues.cc",
        "queues.hpp",
        "nvme.hpp",
        "xts.hpp",
        "xts_tests.cc",
    ],
    deps = [
        ":hal_if",
        ":qstate_if",
        ":r2n",
        ":ssd",
        ":utils",
        "//nic/utils/host_mem",
        "//nic/model_sim:model_client",
    ],
    copts = [
        "-std=c++14",
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
        "//nic:grpc",
    ],
    linkopts = [
        "-lpthread",
        "-pthread",
        "-lz",
        "-lprotobuf",
    ],
)
