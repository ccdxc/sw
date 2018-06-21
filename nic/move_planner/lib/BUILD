package(default_visibility = ["//visibility:public"])

licenses(["notice"]) # MIT license

cc_library(
        name = "pal_lib",
        srcs = ["pal.cc"],
        hdrs = ["pal.h"],
)

cc_library(
        name = "metadata_lib",
        srcs = ["metadata_parser.cc"],
        hdrs = ["metadata_parser.h"],
	deps = [
	   "//nic:boost",
        ],
)

cc_library(
    	name = "planner_lib",
	srcs = ["planner.cc"],
        hdrs = ["planner.h"],
	deps = [
	   ":pal_lib",
	   ":metadata_lib",
	],
)
