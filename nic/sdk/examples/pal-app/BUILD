package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_binary(
    name = "hal_access",
    srcs = [
        "hal_access.cc",
    ],
    deps = [
        "//lib/pal:pal",
        "//lib/logger:logger",
    ],
    linkopts = [
        "-ldl"
    ],
)
