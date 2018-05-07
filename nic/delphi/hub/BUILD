package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "delphi_hub",
    srcs = [
        "delphi_hub_main.cc",
    ],
    deps = [
        "//nic/delphi/utils",
        "//nic/delphi/messanger",
        "//nic/delphi/hub:delphi_server",
        "@third_party//:libev",
    ],
)

cc_library(
    name = "delphi_server",
    srcs = [
    ] + glob(["*.cc"], exclude = ["*_test.cc", "*_main.cc"],) +  glob(["*.hpp"]),
    hdrs = [
        "delphi_server.hpp",
    ],
    include_prefix = "nic/delphi/hub",
    deps = [
        "//nic/delphi/utils",
        "//nic/delphi/messanger",
        "@third_party//:libev_includes",
    ],
)

cc_test(
    name = "delphi_server_test",
    srcs = [
        "delphi_server_test.cc",
    ],
    linkopts = [
        "-lpthread",
    ],
    deps = [
        "//:gtest",
        "//nic/delphi/utils",
        "//nic/delphi/messanger:messanger",
        "//nic/delphi/hub:delphi_server",
        "//nic/delphi/sdk",
        "@third_party//:libev",
    ],
    timeout="short",
)

cc_test(
    name = "delphi_hub_test",
    srcs = [
        "delphi_hub_test.cc",
    ],
    linkopts = [
        "-lpthread",
    ],
    deps = [
        "//:gtest",
        "//nic/delphi/utils",
        "//nic/delphi/messanger:messanger",
        "//nic/delphi/hub:delphi_server",
        "//nic/delphi/sdk",
    ],
    timeout="short",
)
