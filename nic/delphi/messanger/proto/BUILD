package(default_visibility = ["//visibility:public"])

genrule(
    name = "messanger_gen_proto",
    srcs = ["types.proto"],
    outs = [
        "types.pb.cc",
        "types.pb.h",
    ],
    local = 1,
    tools = [
        "//nic/hal/third-party:protoc",
	"//nic/hal/third-party:google_includes",
        "//nic/delphi/proto/delphi:delphi_gen_proto",
    ],
    cmd = "LD_LIBRARY_PATH=nic/hal/third-party/google/x86_64/lib/ $(location //nic/hal/third-party:protoc) -I ./ -I nic/hal/third-party/google/include/ --cpp_out=$(GENDIR) $(SRCS)",
)

cc_library(
    name = "messanger_cc_proto",
    srcs = [
        "types.pb.cc",
        "types.pb.h",
    ],
    deps = [
        "//nic/delphi/proto/delphi:delphi_cc_proto",
    ],
)
