package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

# This dir will be removed once main repo moves to bazel
# and we can directly add 'deps' for nic/model_sim:model_client

cc_library(
    name= "model_client",
    srcs = glob(["*.cc", "*.h"]),
    linkopts = ["-lzmq"],
    hdrs = ["lib_model_client.h"],
)
