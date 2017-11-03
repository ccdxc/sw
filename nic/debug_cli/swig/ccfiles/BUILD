package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "swig_includes",
    hdrs = [
    ] + glob(["*.hpp"]) + glob(["*.h"]),
)

# use cc_binary to force the lib name
cc_binary(
    name = "_p4pd.so",
    linkshared = 1,
    srcs = [
    ] + glob(["*.cc"]),
    deps = [
        "//nic/include:base_includes",
        "@python_path//:python27-lib",
        ":swig_includes",
        "//nic:gen_includes"
    ],
)
