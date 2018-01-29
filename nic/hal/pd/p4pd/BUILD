package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "p4pd_utils",
    srcs = [
        "p4pd_utils.cc",
    ],
    hdrs = glob([
        "*.hpp",
        "*.h",
    ]),
    deps = [
        "//nic/include:base_includes",
        "//nic/hal/pd:hal_pd_includes",
        "//nic:gen_includes",
        "//nic:boost",
    ],
    alwayslink = 1,
)
