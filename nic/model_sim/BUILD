package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "model_client",
    srcs = [
            "src/lib_model_client.cc"
            ],
    linkopts = [
            "-lzmq",
            ],
    deps = [
            "//nic/model_sim/include:modelsim_includes"
            ],
)


cc_binary(
    name = "cap_model",
    srcs = [
            "src/model_server.cc",
            "libs/libmodel_cc.so",
            "libs/libcapsim.a",
            "libs/libcommon_cc.so",
            "libs/libtop_csr_compile.so",
            "libs/libsknobs.so",
            "libs/libasmsym.a",
            "libs/libcapisa.a",
            "libs/libisa.a",
            ],
    deps = [
            "//nic/model_sim/include:modelsim_includes",
            "//nic/utils/host_mem:host_mem_includes",
            "//nic:capricsr_int",
            ],
    linkopts = [
            "-lJudy",
            "-lpython2.7",
            "-lzmq",
            "-lgmp",
            "-lgmpxx",
            "-lpthread",
            "-ldl",
            "-lutil",
            "-lcrypto",
            ],
    )
