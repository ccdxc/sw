package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "tls_api",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
            "//nic:openssl",
            "//nic/hal:hal_includes",
            "//nic/hal/lkl:lklshim_tls",
            "//nic/include:base_includes",
            "//nic/third-party/spdlog",
            "//nic/hal/pd/common:pdcommon",
            "//nic/utils/slab:slab",
            "//nic/utils/indexer:indexer",
            "//nic/utils/catalog:catalog",
            "//nic:gen_proto_includes",
            #"//nic/proto:types_proto",
            #"//nic/proto/hal:hal_proto",
            "//nic:p4pd"
    ],
)
