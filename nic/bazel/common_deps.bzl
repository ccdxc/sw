gl_deps_list = [
        "//nic/hal:hal_src",
        "//nic/hal/src/nw:nw_includes",
        # PI
        "//nic/fte:fte",
        "//nic/hal/periodic",
        "//nic/hal/svc:hal_svc",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/hal/plugins:plugins",
        "//nic/hal/plugins/proxy:proxyplugin",
        "//nic/utils/host_mem:host_mem_src",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/hal/lkl:lkl_api",
        "//nic:lkl",
        "//nic:grpc",
        "//nic:libprotobuf",
        "//nic:halproto",

        # PD
        "@sdk//obj:sdk_catalog",
        "//nic:asic_libs",

        ]

gl_linkopts_list = [
    "-lzmq",
    "-lpthread",
    "-pthread",
    "-lz",
    "-rdynamic",
    "-lm",
    "-lrt",
    ]

sdk_copts = ["-Inic/sdk"]

test_deps_list = [
        "//nic/hal/test/utils:haltestutils",
        # External
        "//:gtest",
        ]

linkmgr_test_deps_list = [
        "//nic/linkmgr/test/utils:linkmgrtestutils",
        # External
        "//:gtest",
        ]
