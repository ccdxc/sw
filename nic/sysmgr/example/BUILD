package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "example",
    srcs = [
        "example.cc"
    ],
    deps = [
        "//nic/sysmgr/proto:sysmgr_gen",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    linkopts = [
        "-lpthread",
    ],
)

# This process is the last to be brought up during the test and send a "SysmgrTestCompleteReq" to the sysmgr.
cc_binary(
    name = "test_complete",
    srcs = [
        "test_complete.cc"
    ],
    deps = [
        "//nic/sysmgr/proto:sysmgr_gen",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    linkopts = [
        "-lpthread",
    ],
)
