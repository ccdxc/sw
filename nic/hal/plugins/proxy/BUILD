package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "proxyplugin",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic:gen_includes",
        "//nic/hal:hal_src",
        "//nic/utils/indexer:indexer",
        "//nic/utils/slab:slab",
        "//nic/utils/ht:ht",
        "//nic/utils/thread:thread",
        "//nic/hal/tls:tls_api",
        "//nic/hal/pd/iris",
        "//nic/hal/lkl:lkl_api",
        #"//nic/fte",
        #"//nic/include:base_includes",
        #"//nic/proto:types_proto",
        #"//nic/proto/agents:agents_proto",
        "//nic/proto/hal:hal_proto",
        #"//nic/third-party/spdlog",
    ],
    alwayslink = 1,
)

cc_library(
    name = "proxyplugin_includes",
 	hdrs = glob(["*.hpp"])
    )
