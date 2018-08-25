package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "iris-c",
    srcs = glob(["src/*.cc"]),
    #srcs = [
    #        "src/hal_client.cc",
    #        "src/hal_grpc_client.cc",
    #        "src/vrf.cc",
    #        "src/l2seg.cc",
    #        "src/uplink.cc",
    #        ],
    hdrs = glob(["include/*.hpp"]),
    deps = [
            "//nic:spdlog",
            "//nic:gen_proto_includes",
            "@sdk//obj:sdk_indexer",
            # TODO: Needed only for arm. Cleanup
            "//nic:grpc_includes",
            "//nic:google_includes",
            "//nic/hal/iris-c/trace"
    ],
    includes = [
            "include",
            ],
)

cc_library(
    name = "iris-c-headers",
    hdrs = glob(["include/*.hpp"]),
    includes = [
            "include",
            ],
)
