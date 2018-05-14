package(default_visibility = ["//visibility:public"])

cc_test(
    name = "messanger_test",
    srcs = [
        "messanger_test.cc",
    ],
    linkopts = [
        "-lpthread",
    ],
    deps = [
        "//:gtest",
        "//nic/delphi/utils",
        "//nic/delphi/messanger:messanger",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    timeout="short",
)

cc_library(
    name = "messanger",
    srcs = [
    ] + glob(["*.cc"], exclude = ["*_test.cc"],) +  glob(["*.hpp"]),
    hdrs = [
        "messanger_client.hpp",
        "messanger_server.hpp",
        "transport.hpp",
    ],
    include_prefix = "nic/delphi/messanger",
    deps = [
        "//nic/delphi/utils",
        "//nic/delphi/messanger/proto:messanger_cc_proto",
        "@third_party//:libev_includes",
        "@third_party//:libev",
    ],
)

cc_test(
    name = "tcp_transport_test",
    srcs = [
        "tcp_transport_test.cc",
    ],
    deps = [
        "//:gtest",
        "//nic/delphi/utils",
        "//nic/delphi/messanger:messanger",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    linkopts = [
        "-lpthread",
    ],
    timeout="short",
)
