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

cc_library(
    name = "libev",

    srcs = select({
        "//conditions:default": [
            "libev/x86_64/lib/libev.a",
        ],

        ":aarch64_mode" : [
            "libev/aarch64/lib/libev.a",
        ],
    }),

    linkopts = select ({
        "//conditions:default" : [
            "-Lthird-party/libev/x86_64",
        ],

        ":aarch64_mode" : [
            "-Lthird-party/libev/aarch64",
        ],
    })
)
cc_library(
    name = "libev_includes",
    hdrs = glob(["libev/include/*.h"]),
    includes = ["libev/include"],
)
