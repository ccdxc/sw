package(default_visibility = ["//visibility:public"])

load("//nic/delphi/compiler:delphi.bzl", "delphi_compile")

cc_binary(
    name = "delphictl",
    srcs = [
        "delphictl_main.cc",
        "argh/argh.h",
    ],
    deps = [
        "//nic/delphi/sdk",
        "//nic/delphi/example:example_gen",
        "//nic/upgrade_manager/upgrade:upgrade_gen",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    linkopts = [
        "-lpthread",
    ],
)
