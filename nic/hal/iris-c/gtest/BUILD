# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_test(
    name = "filter_test",
    srcs = ["filter_test.cc"],
    deps = [
            # Internal
            "//nic/hal/iris-c:iris-c",
            #"//nic/hal/iris-c:iris-c-headers",
            "//nic:halproto",
            "//nic:libprotobuf",
            "//nic:grpc",
            "@sdk//obj:sdk_logger",
            # External
            "//:gtest",
            ],
    linkopts = [
            "-lz",
            ],
)

cc_test(
    name = "filter_smart_test",
    srcs = ["filter_smart_test.cc"],
    deps = [
            # Internal
            "//nic/hal/iris-c:iris-c",
            #"//nic/hal/iris-c:iris-c-headers",
            "//nic:halproto",
            "//nic:libprotobuf",
            "//nic:grpc",
            "@sdk//obj:sdk_logger",
            # External
            "//:gtest",
            ],
    linkopts = [
            "-lz",
            ],
)
