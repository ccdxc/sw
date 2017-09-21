package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name= "hal_if",
    srcs = [
        "hal_if.cc",
        "hal_if.hpp",
    ],
    deps = [
        "//dol/test/storage/proto:hal_proto",
        "//external:gflags",
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
        "//dol/test/storage/host_mem",
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
    ],
    deps = [
        ":hal_if",
        ":qstate_if",
        ":r2n",
        "//dol/test/storage/host_mem",
        "//dol/test/storage/model_client",
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
        "//external:gflags",
    ],
)
