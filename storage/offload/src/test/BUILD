package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pnso_test_sim",
    srcs = [
        "pnso_test.c",
        "pnso_test_file.c",
        "pnso_test_parse.c",
    ],
    hdrs = [
        "pnso_test.h",
        "pnso_test_parse.h",
        "pnso_test_desc.h",
        "pnso_test_ctx.h",
    ],
    includes = [
        ".",
        "../common",
    ],
    deps = [
        "//storage/offload/third-party:yaml",
        "//storage/offload/src/sim:pnso_sim",
        "//storage/offload/src/osal:osal",
        "//storage/offload:pnso_pbuf",
    ],
    copts = [
        "-xc",
	"-DPNSO_SIM"
    ],
    #linkopts = [ "-lyaml" ],
    alwayslink = 1,
    linkstatic = 1,
)

cc_binary(
    name = "pencake_sim",
    srcs = [ "pnso_test_main.c" ],
    deps = [
        "pnso_test_sim",
    ],
    copts = [
        "-xc"
    ],
    linkopts = [ "-lpthread", "-lrt", "-ldl", "-lnuma", "-luuid" ],
)
