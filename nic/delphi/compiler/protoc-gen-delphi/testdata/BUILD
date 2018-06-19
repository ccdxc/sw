package(default_visibility = ["//visibility:public"])

load("//nic/delphi/compiler:delphi.bzl", "delphi_compile")

# compile the test proto file to verify compiler works fine
delphi_compile(
    name = "test_gen",
    srcs = [
      'test.proto',
    ],
)
