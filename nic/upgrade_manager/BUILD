package(default_visibility = ["//visibility:public"])

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")
load("//nic/delphi/compiler:delphi.bzl", "delphi_compile")

# compile the proto file
delphi_compile(
    name = "upgrade_gen",
    srcs = [ 'proto/upgrade.proto' ],
)

cc_library(
    name = "upgrade_app_lib_includes",
    srcs = [],
    hdrs = ["export/upgrade.hpp"],
)

# compile the reactor for apps
cc_library(
    name = "upgrade_app_lib",
    srcs = [
        "export/upgrade.cc",
        "export/upgrade.hpp",
    ],
    hdrs = [
        "export/upgrade.hpp",
    ],
    deps = [
        "//nic/delphi/sdk",
        ":upgrade_gen",
    ],
)

# compile the service and reactor
cc_library(
    name = "upgrade_lib",
    srcs = [
        "lib/upgrade.cc",
        "lib/upgrade.hpp",
    ],
    hdrs = [
        "lib/upgrade.hpp",
    ],
    deps = [
        "//nic/delphi/sdk",
        ":upgrade_gen",
    ],
)

# Compile the upgrade binary
cc_binary(
    name = "upgrade",
    srcs = [
        "src/upgrade_main.cc"
    ],
    deps = [
        ":upgrade_lib",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
)


#################################
############ EXAMPLE ############ 
#################################
# compile the example service library
cc_library(
    name = "example_lib",
    srcs = [
        "example/example.cc",
        "example/example.hpp",
    ],
    hdrs = [
        "example/example.hpp",
    ],
    deps = [
        "//nic/delphi/sdk",
        ":upgrade_app_lib",
        "//nic/upgrade_manager:upgrade_app_lib_includes",
    ],
)

# Compile the example binary
cc_binary(
    name = "example",
    srcs = [
        "example/example_main.cc"
    ],
    deps = [
        ":example_lib",
        ":upgrade_app_lib",
        "//nic/upgrade_manager:upgrade_app_lib_includes",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
)

# run the tests
cc_test(
    name = "upgrade_manager_test",
    srcs = [
        "test/upgrade_manager_test.cc",
    ],
    linkopts = [
        "-lpthread",
    ],
    deps = [
        "//:gtest",
        ":upgrade_lib",
        "//nic/delphi/sdk",
        "@third_party//:libev",
        "@third_party//:libev_includes",
    ],
    timeout="short",
)

