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
    copts = [
        "-xc"
    ],
    hdrs = glob(["*.h"]),
)

cc_binary(
    name = "osal_test",
    srcs = glob(["test/*.h"]) +
           glob(["test/*.c"]), 
    deps = [
        "//storage/offload/src/sim:pnso_sim",
        "osal",
    ],
    copts = [ "-g" ],
    linkopts = [ "-lpthread"],
)
