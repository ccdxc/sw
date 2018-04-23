package(default_visibility = ["//visibility:public"])

cc_library(
    name = "sdk",
    srcs = [
    ] + glob(["*.cc"], exclude = ["*_test.cc"],) +  glob(["*.hpp"]),
    hdrs = [
        "delphi_sdk.hpp",
        "delphi_client.hpp",
        "base_object.hpp",
        "delphi_service.hpp",
        "object_factory.hpp",
        "delphi_utest.hpp",
    ],
    include_prefix = "nic/delphi/sdk",
    deps = [
        "//nic/delphi/utils",
        "//nic/delphi/messanger",
    ],
    linkopts = [
        "-lev",
        "-L/usr/local/lib",
    ],
)

cc_test(
    name = "delphi_client_test",
    srcs = [
        "delphi_client_test.cc",
    ],
    linkopts = [
        "-lpthread",
        "-lev",
        "-L/usr/local/lib",
    ],
    deps = [
        "//:gtest",
        "//nic/delphi/utils",
        "//nic/delphi/messanger:messanger",
        "//nic/delphi/sdk",
    ],
    timeout="short",
)
