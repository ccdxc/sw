package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pnso_sim",
    srcs = glob(["src/sim/*.h"]) +
        glob(["src/sim/*.c"]) + [
        "src/osal/pnso_sim_osal.h",
#        "src/osal/linux/uspace/pnso_sim_uspace.c",
    ],
    includes = [
        "include",
        "src/sim",
        "src/osal"
    ],
    deps = [ "//nic:openssl-native" ],
    copts = [
        "-xc"
    ],
    hdrs = [
        "include/pnso_sim_api.h",
        "include/pnso_wafl.h",
        "src/sim/pnso_sim.h"
    ],
    alwayslink = 1
)

cc_library(
    name = "spdk",
    deps = ["//storage/offload/third-party:spdk"],
)

cc_binary(
    name = "pnso_offloader",
    srcs = [
	"src/linux/uspace/offloader.c",
	"src/linux/uspace/offloader.h",
	"src/linux/uspace/io_initiator.c",
	"src/linux/uspace/eff.c",
	"src/linux/uspace/xor.c",
	"src/linux/uspace/read.c",
	],
    deps = [
	"//storage/offload:spdk",
	"pnso_sim",
	"osal",
	],
    copts = [ "-g" ],
    linkopts = [ "-lpthread", "-lrt", "-ldl", "-lnuma", "-luuid"],
)

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
    srcs = glob(["src/osal/*.h"]) +
           glob(["src/osal/*.c"]) + 
#need to delete below lines once pnso_sim_uspace.c is moved out of osal
           glob(["src/sim/*.h"]) + 
           glob(["include/*.h"]) + 
#need to delete above lines once pnso_sim_uspace.c is moved out of osal
	   select({
		"//conditions:default": 
			#glob(include=["src/osal/linux/uspace/*.c"],
                        #     exclude=["src/osal/linux/uspace/pnso_sim_uspace.c"]) +
			glob(["src/osal/linux/uspace/*.c"]) +
			glob(["src/osal/linux/uspace/*.h"]) +
			glob(["src/osal/linux/uspace/sim/*.c"]) + 
			glob(["src/osal/linux/uspace/sim/*.h"]),
		":user_dol" : [
		],
	   }),
    includes = [
        "include",
        "src/sim",
        "src/osal"
    ],
    copts = [
        "-xc"
    ],
    hdrs = glob(["src/osal/*.h"])
)

cc_binary(
    name = "osal_test",
    srcs = glob(["src/osal/test/*.h"]) +
           glob(["src/osal/test/*.c"]), 
    includes = [
        "include",
        "src/sim",
        "src/osal"
    ],
    deps = [
	"pnso_sim",
	"osal",
	],
    copts = [ "-g" ],
    linkopts = [ "-lpthread"],
)
