# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "tcam",
    srcs = glob(["*.cc"]) + ["tcam_entry.hpp"],
    hdrs = glob(["tcam.hpp"]),
    deps = [
            "//:sdk_headers",
            "//lib/p4pd:p4pd_api",
    ],
)
