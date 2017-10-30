package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "isc-dhcp",
    srcs = ["dhcp_pen_api.c",
	    "common/alloc.c",
	    "common/options.c",
	    "common/parse.c",
	    "common/tables.c",
	    "common/tree.c",
            "omapip/alloc.c",
            "omapip/errwarn.c",
	    "omapip/hash.c"],

    hdrs = ["includes/omapip/alloc.h",
            "includes/omapip/hash.h",
            "includes/dhcp.h",
            "includes/dhcp6.h",
            "includes/dhcp_internal.h",
            "includes/tree.h",
	   ],
    copts = ["-x c++", "--std=c++11"],
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
    ],
)
