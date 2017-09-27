package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "fte",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = ["//nic/include:base_includes", 
            "//nic/hal:hal_src",
            "//nic/hal/svc:hal_svc",
            "//nic/proto:types_proto",
            "//nic/proto/hal:hal_proto",
            "//nic/proto/agents:agents_proto",
            "//nic:gen_includes",
            "//nic/p4/nw/include:p4_nw_includes",
            "//nic/third-party/spdlog:spdlog"],
    alwayslink = 1,
)
