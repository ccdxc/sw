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
         + glob(["include/sdk/*.h"])
         + glob(["include/sdk/asic/capri/csrlite/*.hpp"])
         + glob(["include/sdk/asic/capri/csrlite/*.h"])
         + glob(["gen/include/sdk/asic/capri/csrlite/*.hpp"]),
)

cc_library(
    name = "csrlite_gen_includes",
    includes = ["include", "gen/include"],
    hdrs = glob(["include/sdk/asic/capri/csrlite/*.hpp"])
         + glob(["include/sdk/asic/capri/csrlite/*.h"])
         + glob(["gen/include/sdk/asic/capri/csrlite/*.hpp"]),
)
