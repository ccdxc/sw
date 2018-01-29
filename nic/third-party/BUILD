# Indirection for all external dependencies.
package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "boost",
    includes = ["boost/include"],
    hdrs = glob(["boost/include/**/*.hpp"]) + glob(["boost/include/**/*.h"] + glob(["boost/include/**/*.ipp"])),
)

config_setting(
    name = "aarch64_mode",
    values = { "cpu": "aarch64-linux" }
)
