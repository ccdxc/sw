package(default_visibility = ["//visibility:public"])

cc_library(
    name = "headers",
    srcs = [],
    hdrs = [
        "delphi_sdk.hpp",
        "delphi_client.hpp",
        "base_object.hpp",
        "delphi_service.hpp",
        "object_factory.hpp",
        "delphi_utest.hpp",
        "delphi_metrics.hpp",
    ],
    deps = [
        "//nic/delphi/utils",
        "//nic/delphi/messanger",
        "//nic/delphi/shm",
        "@third_party//:libev_includes",
    ]
)

cc_library(
    name = "sdk",
    srcs = [
    ] + glob(["*.cc"], exclude = ["*_test.cc"],) +  glob(["*.hpp"]),
    hdrs = [
        "delphi_sdk.hpp",
        "delphi_client.hpp",
        "base_object.hpp",
        "delphi_service.hpp",
        "object_factory.hpp",
        "delphi_utest.hpp",
        "delphi_metrics.hpp",
        "delphi_metrics_cgo.h",
    ],
    include_prefix = "nic/delphi/sdk",
    deps = [
        "headers",
        "//nic/delphi/utils",
        "//nic/delphi/messanger",
        "//nic/delphi/shm",
        "//nic/delphi/sdk/proto:client",
        "//nic/delphi/proto/delphi:delphi_cc_proto",
        "@third_party//:libev_includes",
        "@third_party//:libev",
    ],
)

cc_test(
    name = "delphi_client_test",
    srcs = [
        "delphi_client_test.cc",
    ],
    linkopts = [
        "-lpthread",
        "-lm",
    ],
    deps = [
        "//:gtest",
        "//nic/delphi/utils",
        "//nic/delphi/messanger:messanger",
        "//nic/delphi/sdk",
        "@third_party//:libev",
    ],
    timeout="short",
)

cc_test(
    name = "delphi_metrics_test",
    srcs = [
        "delphi_metrics_test.cc",
    ],
    linkopts = [
        "-lpthread",
        "-lrt",
        "-lm",
    ],
    deps = [
        "//:gtest",
        "//nic/delphi/utils",
        "//nic/delphi/shm",
        "//nic/delphi/sdk",
        "@third_party//:libev",
    ],
    timeout="short",
)
