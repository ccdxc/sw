# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "hash",
    srcs = glob(["*.cc"]) + glob(["*.hpp"]),
    hdrs = [
        "hash.hpp",
        "hash_entry.hpp",
    ],
    deps = [
            "//:sdk_headers",
            "//lib/p4pd:p4pd_api",
            "@third_party//:boost",
    ],
)
