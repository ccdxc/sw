package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "csr_lite_base_includes",
    srcs = [],
    hdrs = ["include/cap_csr_base.hpp"],
    includes = ["include"],
    deps = [
            "//nic:csr_lite_asic_includes",
            "//nic/hal/pd/capri/csr:sknobs_hdrs"
           ],
)

cc_library(
    name = "csr_lite",
    srcs = glob(["*.cc"]),
    hdrs = glob(["include/*.hpp"]),
    includes = ["include"],
    deps = [
            "//nic/gen/csr_lite:csr_lite_gen",
           ],
)

