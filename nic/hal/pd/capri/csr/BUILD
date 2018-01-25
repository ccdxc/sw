package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

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
    name = "hal_pknobs",
    hdrs = [
        "hal_pknobs.hpp"
    ],
    srcs = [
        "hal_pknobs.cc",
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
        "cpu_hal_if.h",
    ],
    deps = [
        "//nic:capricsr_int",
        "//nic/include:base_includes",
        "//nic:spdlog",
    ],
    copts = sdk_copts,
)
