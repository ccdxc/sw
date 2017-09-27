package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "sknobs_hdrs",
    srcs = [
        "sknobs.h",
        "pknobs_reader.h",
        "import_export.hpp",
    ],
    includes = ["."],
)

cc_library(
    name = "capricsr",
    srcs = [
        "cpu_hal_if.cc",
        "cpu_hal_if.h",
    ],
    hdrs = [
        "cpu_hal_if.h"
    ],
    deps = [
        "//nic:capricsr_int",
    ],
)
