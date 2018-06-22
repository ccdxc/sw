package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pnso_sim",
    srcs = glob(["src/sim/*.h"]) +
        glob(["src/sim/*.c"]),
    includes = [
        "include",
        "src/sim",
        "src/osal",
        "third-party/freebsd-crypto/src"
    ],
    #deps = [ "//nic:openssl-native" ],
    deps = [
        "osal",
        "//storage/offload/third-party:sim_crypto"
    ],
    copts = [
        "-xc"
    ],
    hdrs = glob(["src/osal/osal*.h"]) +
        glob(["include/pnso*.h"]),
    alwayslink = 1
)

cc_library(
    name = "pnso_test_sim",
    srcs = [
        "src/test/pnso_test.h",
        "src/test/pnso_test_parse.h",
        "src/test/pnso_test_parse.c",
        "src/test/pnso_test_file.c",
        "src/test/pnso_test.c"
    ],
    includes = [
        "include",
        "src/osal"
    ],
    deps = [
        "//storage/offload/third-party:yaml",
        "pnso_sim",
        "osal",
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
    name = "pnso_test_main",
    srcs = [ "src/test/pnso_test_main.c" ],
    includes = [
        "src/test"
    ],
    deps = [
        "//storage/offload:pnso_test_sim",
    ],
    copts = [
        "-xc"
    ],
    linkopts = [ "-lpthread", "-lrt", "-ldl", "-lnuma", "-luuid" ],
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
    srcs = glob(["src/osal/*.c"]) + 
	   select({
		"//conditions:default": 
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
