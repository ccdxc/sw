package(default_visibility = ["//visibility:public"])

cc_test(
    name = "messanger_test",
    srcs = [
        "messanger_test.cc",
    ],
    linkopts = [
#        "-lprotobuf",
        "-lpthread",
        "-lev",
        "-L/usr/local/lib",
    ],
    deps = [
        "//:gtest",
        "//nic/delphi/utils",
        "//nic/delphi/messanger:messanger",
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
        "//nic/delphi/messanger/proto:types_cc_proto",
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
    ],
    linkopts = [
        "-lpthread",
        "-lev",
        "-L/usr/local/lib",
    ],
    timeout="short",
)
