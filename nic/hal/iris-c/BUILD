package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "iris-c",
    srcs = glob(["src/*.cc"]),
    hdrs = glob(["include/*.hpp"]),
    deps = [
            "//nic:spdlog",
            "//nic:gen_proto_includes",
            "@sdk//obj:sdk_indexer",
            # TODO: Needed only for arm. Cleanup
            "//nic:grpc_includes",
            "//nic:google_includes",
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
