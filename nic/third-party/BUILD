# Indirection for all external dependencies.
package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "boost",
    includes = ["boost/include"],
    hdrs = glob(["boost/include/**/*.hpp"]) + glob(["boost/include/**/*.h"] + glob(["boost/include/**/*.ipp"])),
)

cc_library(
    name = "libprotobuf",
    srcs = [
	"google/x86_64/lib/libprotobuf.so.14",
    ],
)

cc_library(
    name = "libtirpc",
    hdrs = glob(["libtirpc/export/*.h"]),

    includes = [
        "libtirpc/export"
    ],
)

cc_library(
    name = "google_includes",
    hdrs = glob(["google/include/**/*.hpp"]) + glob(["google/include/**/*.h"]),
    includes = ["google/include/"],
)

cc_library(
    name = "grpc",

    srcs = select({
        "//conditions:default": [
            "grpc/x86_64/lib/libgrpc++_reflection.a",
            "grpc/x86_64/lib/libgrpc++.a",
            "grpc/x86_64/lib/libgrpc_unsecure.a",
            "grpc/x86_64/lib/libgrpc++_unsecure.a",
        ],

        ":aarch64_mode" : [
            "grpc/aarch64/lib/libgrpc++_reflection.a",
            "grpc/aarch64/lib/libgrpc++.a",
            "grpc/aarch64/lib/libgrpc_unsecure.a",
            "grpc/aarch64/lib/libgrpc++_unsecure.a",
        ],
    }),

    linkopts = select ({
        "//conditions:default" : [
            "-Lthird-party/grpc/x86_64",
        ],

        ":aarch64_mode" : [
            "-Lthird-party/grpc/aarch64",
        ],
    })
)

cc_library(
    name = "grpc_includes",
    hdrs = glob(["grpc/include/**/*.hpp"]) + glob(["grpc/include/**/*.h"]),
    includes = ["grpc/include"],
)

config_setting(
    name = "aarch64_mode",
    values = { "cpu": "aarch64-linux" }
)
