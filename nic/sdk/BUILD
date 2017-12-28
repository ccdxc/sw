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
    includes = ["include"],
    srcs = [],
    hdrs = glob(["include/sdk/*.hpp", "include/sdk/*.h"]),
)
