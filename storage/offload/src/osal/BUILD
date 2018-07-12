# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

#This is the default
config_setting(
    name = "user_sim",
    values = { "define": "mode=user_sim" }	
)

config_setting(
    name = "user_dol",
    values = { "define": "mode=user_dol" }	
)

config_setting(
    name = "kernel_sim",
    values = { "define": "mode=kernel_sim" }	
)

cc_library(
    name = "osal",
    srcs = glob(["*.c"]) + 
        select({
            "//conditions:default": 
                glob(["linux/uspace/*.c"]) +
                glob(["linux/uspace/*.h"]) +
                glob(["linux/uspace/sim/*.c"]) + 
                glob(["linux/uspace/sim/*.h"]),
            ":user_dol" : [
            ],
        }),
    includes = [
        ".",
    ],
    deps = [
        "//storage/offload/include:pnso",
    ],
    copts = [
        "-xc"
    ],
    hdrs = glob(["*.h"]),
)

cc_binary(
    name = "osal_test",
    srcs = [
	"test/osal_test.c",
    ],
    deps = [
        "//storage/offload/src/sim:pnso_sim",
        "osal",
    ],
    copts = [ "-g" ],
    linkopts = [ "-lpthread"],
)

cc_test(
    name = "osal_logger_test",
    srcs = [
        "test/osal_logger_test.c",
    ],
    copts = [
        "-g",
    ],
    deps = [
        "//storage/offload/include:pnso",
        "//storage/offload:osal",
    ],
    linkopts = [
	"-lpthread"
    ],
)
