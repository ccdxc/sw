# Indirection for all external dependencies.
package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "gtest",
    visibility = ["//visibility:public"],
    deps = [
            "@gtest//:gtest",
            ],
)

cc_library(
    name = "sdk_headers",
    includes = ["include", "gen/include"],
    hdrs = glob(["include/sdk/*.hpp"])
         + glob(["include/sdk/*.h"]),
)

cc_library(
    name = "csrlite_gen_includes",
    includes = ["include", "gen/include"],
    hdrs = glob(["include/sdk/asic/capri/csrlite/*.hpp"])
         + glob(["include/sdk/asic/capri/csrlite/*.h"])
         + glob(["gen/include/sdk/asic/capri/csrlite/*.hpp"]),
)

# TODO remove later
cc_library(
    name = "csr_includes",
    includes = [ "include/sdk/asic/capri" ],
    hdrs = glob(["include/sdk/asic/capri/*.hpp"])
         + glob(["include/sdk/asic/capri/*.h"]),
)

cc_library(
    name = "aapl_includes",
    hdrs = glob(["third-party/avago/include/aapl/*.h"])
)

cc_library(
    name = "libaapl",
    srcs = select ({
        "//conditions:default" : [
            "third-party/avago/x86_64/lib/aapl/C/libAAPL.so",
        ],
        ":aarch64_mode" : [
            "third-party/avago/aarch64/lib/aapl/C/libAAPL.so",
        ],
    }),
)

config_setting(
    name = "aarch64_mode",
    values = { "cpu": "aarch64-linux" }
)

