package(default_visibility = ["//visibility:public"])

cc_test(
    name = "planner_unit_test",
    srcs = [
        "planner_unit_test.cc",
    ],
    linkopts = [
        "-lpthread",
    ],
    deps = [
        "//nic/move_planner/lib:pal_lib",
        "//nic/move_planner/lib:metadata_lib",
        "//nic/move_planner/lib:planner_lib",
        "//:gtest",
    ],
    timeout="short",
)
