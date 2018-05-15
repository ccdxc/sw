package(default_visibility = ["//visibility:public"])

licenses(["notice"]) # MIT license

cc_library(
	name = "malloc_stub",
    	srcs = ["malloc_stub.cc"],
	hdrs = ["malloc_stub.h"],
)

cc_library(
        name = "mom_lib",
        srcs = ["mom.cc"],
        hdrs = ["mom.h"],
	deps = [
	       ":malloc_stub",
	],
)

cc_binary(
    	name = "moveable_objects",
	srcs = ["mom_test.cc"],
	deps = [
	   ":malloc_stub",
	   ":mom_lib",
	],
)
