package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "libevutils",
    srcs = [
        "fd.c",
        "run.c",
        "stop.c",
    ],
    hdrs = [
    ],
    # libev has some code that trips alias check, so -Wno-strict-aliasing
    copts = [
        "-g",
	"-Wno-strict-aliasing",
	"-Iplatform/src/lib/evutils/include",
	"-Inic/third-party/libev/include",
    ],
    deps = [
        "//platform/src/lib/evutils/include:libevutils_hdrs",
        "//nic/third-party:libev_includes",
    ],
    alwayslink = True,
)

