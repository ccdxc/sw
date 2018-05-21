package(default_visibility = ["//visibility:public"])


# compile the service and reactor
cc_library(
    name = "hal_svc",
    srcs = [
    ] + glob(["*.cc"], exclude = ["*_test.cc", "*_main.cc"],) +  glob(["*.hpp"]),
    deps = [
        "//nic/delphi/sdk",
        "//nic/delphi/example/hal/proto:hal_svc_gen",
    ],
)

# Compile the example hal binary
cc_binary(
    name = "hal",
    srcs = [
        "hal_main.cc"
    ],
    deps = [
        ":hal_svc",
        "//nic/delphi/sdk",
    ],
    linkopts = [
        "-lev",
        "-L/usr/local/lib",
    ],
)
