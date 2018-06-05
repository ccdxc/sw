# Indirection for all external dependencies.
package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

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

cc_library(
    name = "libprotobuf",
    srcs = select ({
        "//conditions:default" : [
	        "google/x86_64/lib/libprotobuf.so.14",
        ],
        ":aarch64_mode" : [
	        "google/aarch64/lib/libprotobuf.so.14",
        ],
    }),
)

cc_library(
    name = "google_includes",
    hdrs = glob(["google/include/**/*.hpp"]) + glob(["google/include/**/*.h"]),
    includes = ["google/include/"],
)

cc_library(
    name = "isc_includes",
    srcs = [],
    hdrs = glob(["isc/include/**/*.hpp", "isc/include/**/*.h"]),
    includes = ["isc/include/"],
)

cc_library(
    name = "libtirpc",
    hdrs = glob(["libtirpc/export/*.h"]),

    includes = [
        "libtirpc/export"
    ],
)

cc_library(
    name = "libtins",
    srcs = select ({
        "//conditions:default" : [
	        "tins/x86_64/lib/libtins.so",
        ],
        ":aarch64_mode" : [
            "tins/aarch64/lib/libtins.so",
            "pcap/aarch64/lib/libpcap.so.0.8",
        ],
    }),
)

cc_library(
   name = "spdlog",
   includes = ["spdlog/include"],
   hdrs = glob(["spdlog/include/**/*.hpp"]) + glob(["spdlog/include/**/*.h"]) + glob(["spdlog/include/**/*.cc"]),
)

cc_library(
    name = "snort3",
    srcs = select({
        "//conditions:default": [
            "snort3/export/x86_64/lib/libsnort.so",
        ],
        ":aarch64_mode": [
            "snort3/export/aarch64/lib/libsnort.so",
        ],
    }),
    hdrs = glob(["snort3/export/include/*.h"]),
    includes = [
        "snort3/export/include",
    ],
)

cc_library(
    name = "lkl_includes",

    srcs = [
        "lkl/export/include/lkl.h",
        "lkl/export/include/lkl_host.h",
    ],
    hdrs = [
       "lkl/export/include/lkl.h",
       "lkl/export/include/lkl_host.h",
       "lkl/export/include/lkl_autoconf.h",
       "lkl/export/include/lkl/autoconf.h",
       #"third-party/lkl/export/include/lkl/asm/syscalls.h",
       #"third-party/lkl/export/include/lkl/linux/types.h",
       #"third-party/lkl/export/include/lkl/linux/stddef.h",
       #"third-party/lkl/export/include/lkl/linux/posix_types.h",
       #"third-party/lkl/export/include/lkl/asm/types.h",
       #"third-party/lkl/export/include/lkl/asm/posix_types.h",
       #"third-party/lkl/export/include/lkl/asm/bitsperlong.h",
       #"third-party/lkl/export/include/lkl/asm-generic/types.h",
       #"third-party/lkl/export/include/lkl/asm-generic/posix_types.h",
       #"third-party/lkl/export/include/lkl/asm-generic/int-ll64.h",
    ] + glob(["lkl/export/include/lkl/linux/*.h",
              "lkl/export/include/lkl/asm/*.h",
              "lkl/export/include/lkl/asm-generic/*.h",
              "lkl/export/include/lkl/linux/hdlc/*.h",
              "lkl/export/include/lkl/linux/byteorder/*.h"]),

    includes = [
        "lkl/export/include",
    ],
)

cc_library(
    name = "openssl",
    hdrs = glob(["openssl/include/openssl/*.h"]),

    srcs = select ({
        "//conditions:default" : [
            "openssl/x86_64/lib/libssl.so",
            "openssl/x86_64/lib/libssl.so.1.1",
            "openssl/x86_64/lib/libcrypto.so",
            "openssl/x86_64/lib/libcrypto.so.1.1",
        ],
        ":aarch64_mode" : [
            "openssl/aarch64/lib/libssl.so",
            "openssl/aarch64/lib/libssl.so.1.1",
            "openssl/aarch64/lib/libcrypto.so",
            "openssl/aarch64/lib/libcrypto.so.1.1",
        ],
    }),

    includes = [
        "openssl/include",
    ],

    linkopts = select ({
        "//conditions:default" : [
            "-Lopenssl/x86_64/lib -ldl",
        ],
        ":aarch64_mode" : [
            "-Lopenssl/aarch64/lib -ldl",
        ],
    }),
)

cc_library(
    name = "openssl-native",
    hdrs = glob(["openssl-native/include/openssl/*.h"]),

    srcs = [
        "openssl-native/lib/libssl.a",
        "openssl-native/lib/libcrypto.a",
    ],

    includes = [
        "openssl-native/include",
    ],

    linkopts = [
        "-Lopenssl-native/lib -ldl",
    ],
)

cc_library(
    name = "lkl",

    srcs = select ({
        "//conditions:default" : [
            "lkl/export/x86_64/bin/liblkl.so",
        ],

        ":aarch64_mode" : [
            "lkl/export/aarch64/bin/liblkl.so",
        ],
    }),

    linkopts = select ({
        "//conditions:default" : [
            "-Llkl/export/x86_64/bin"
        ],

        ":aarch64_mode" : [
            "-Llkl/export/aarch64/bin"
        ],
    }),
)

filegroup(
    name = "protoc",
    srcs = [
        "google/x86_64/bin/protoc",
    ],
)

config_setting(
    name = "aarch64_mode",
    values = { "cpu": "aarch64-linux" }
)
