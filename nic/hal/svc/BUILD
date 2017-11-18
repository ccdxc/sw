package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "hal_svc",
    srcs = [
            ] + glob(["*.cc"]),
    hdrs = [
            ] + glob(["*.hpp"]),
    deps = ["//nic/include:base_includes",
            "//nic:gen_includes",
            "//nic:gen_proto_includes",
            "//nic/hal:hal_src",
            "//nic/third-party/spdlog:spdlog"],
)
