package(default_visibility = ["//visibility:public"])

# Compile the upgrade binary
cc_binary(
    name = "upgrade",
    srcs = [
        "upgrade_main.cc"
    ],
    deps = [
        "//nic/upgrade_manager/lib:upgrade_lib",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    linkopts = [
        "-lpthread",
    ],
)
