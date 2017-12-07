package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "hal_handle_includes",
    hdrs = [
        "hal_handle.hpp",
    ],
)

cc_library(
    name = "hal_handle",
    srcs = [
        "hal_handle.cc",
    ],
    hdrs = [
        "hal_handle.hpp",
    ],
    deps = [
        "//nic/utils/list:list_includes",
        "//nic/utils/ht:ht",
        "//nic/utils/slab:slab",
    ],
)
