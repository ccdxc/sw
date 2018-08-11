gl_deps_list = [
    "//nic/hal:hal_src",
    "//nic/hal/plugins/cfg/nw:nw_includes",
    "//nic/hal/plugins/cfg/aclqos:aclqos_includes",
    "//nic/hal/plugins/cfg/dos:dos_includes",
    "//nic/hal/plugins/cfg/l4lb:l4lb_includes",
    # PI
    "//nic/fte:fte",
    "//nic/hal/core:periodic",
    "//nic/hal/svc:hal_svc",
    "//nic/utils/trace",
    "//nic/utils/print",
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
    "//nic:libsknobs",
]

gl_linkopts_list = [
    "-lzmq",
    "-lpthread",
    "-pthread",
    "-lz",
    "-rdynamic",
    "-lm",
    "-lrt",
    "-lJudy",
    ]

sdk_copts = ["-Inic/sdk"]

csr_copts = [
    # "-Os",

    # Disabling warnings
    "-Wno-unused-function",
    "-Wno-unused-variable",
    "-Wno-sign-compare",
    "-Wno-maybe-uninitialized",
    "-Wno-uninitialized",
    "-Wno-unused-but-set-variable",
    "-fno-asynchronous-unwind-tables",

    # TODO enable after fixing issues with cc_test
    # "-fvisibility=hidden",
    # "-fno-exceptions",
]

csr_defines_copts = [
    "-DEXCLUDE_PER_FIELD_CNTRL",
    "-DCAPRI_HAL",
    "-DCAP_CSR_LARGE_ARRAY_THRESHOLD=1024",
    "-DBOOST_EXCEPTION_DISABLE",
    "-DCSR_NO_SHOW_IMPL",
    "-DCSR_NO_CALLBACK",
    "-DDCSR_NO_RESET_VAL",
    "-DCSR_NO_INST_PATH",
    "-DCSR_NO_CTOR_DEF_NAME",
    "-DCSR_NO_BASE_NAME",
    "-DCSR_NO_CSR_TYPE",
    "-DPLOG_ONLY_ERR_MODE",
]

test_deps_list = [
    "//nic/hal/test/utils:haltestutils",
    "//nic:hal_svc_gen",
    # External
    "//:gtest",
]
