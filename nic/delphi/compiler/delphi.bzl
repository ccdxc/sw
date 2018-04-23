# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

""" Rules for delphi compiler """

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")

def delphi_compile(name, srcs, ):
    genfiles = []
    for src in srcs:
        # Verify we are dealing with proto files
        ext = src.split(".")[1]
        if ext != "proto":
            print("File " + src + " is not a .proto file")
            fail("File " + src + " is not a .proto file")

        fname = src.split(".")[0]
        genfiles.append(fname + ".pb.cc")
        genfiles.append(fname + ".pb.h")
        genfiles.append(fname + ".delphi.cc")
        genfiles.append(fname + ".delphi.hpp")
        genfiles.append(fname + ".delphi_utest.hpp")

    native.genrule(
        name = name + '_delphi_gen',
        srcs = srcs,
        outs = genfiles,
        local = 1,
        tools = [
            "//nic/delphi/compiler:delphi_compiler",
            "//nic/delphi/proto/delphi:delphi_cc_proto",
        ],
        cmd = "/usr/bin/python $(location //nic/delphi/compiler:delphi_compiler) -outdir $(GENDIR)/ -workspace ./ -input $(SRCS) ",

    )

    native.cc_library(
        name = name,
        srcs = genfiles,
        deps = [
            "//nic/delphi/sdk",
        ],
    )
