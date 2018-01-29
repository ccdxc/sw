gl_deps_list = [
        "//nic/hal:hal_src",
        # PI
        "//nic/fte:fte",
        "//nic/hal/periodic",
        "//nic/hal/svc:hal_svc",
        "//nic/utils/trace",
        "@sdk//obj:sdk_logger",
        "//nic/utils/print",
        "//nic/hal/plugins:plugins",
        "//nic/hal/plugins/proxy:proxyplugin",
        "//nic/utils/host_mem:host_mem",
        "//nic:gen_proto_includes",
        "//nic/hal/test/utils:haltestutils",
        "//nic/hal/lkl:lkl_api",
        "//nic:lkl",
        "//nic:grpc",
        "//nic:libprotobuf",
        "//nic:halproto",

        # PD
        "//nic/hal/pd:pdcommon",
        "@sdk//obj:sdk_tcam",
        "@sdk//obj:sdk_hash",
        "@sdk//obj:sdk_directmap",
        "//nic/hal/pd/utils/flow",
        "//nic/hal/pd/utils/met",
        "//nic/hal/pd/utils/acl_tcam",
        "//nic/hal/pd/iris",
        "//nic:capricsr_int",
        "//nic/hal/pd/capri",
        "@sdk//obj:sdk_pal",
        "//nic/model_sim:model_client",
        "//nic:p4pd",
        "//nic:p4pluspd_txdma",
        "//nic:p4pluspd_rxdma",
        "//nic:asic_libs",
        "//nic/hal/lib:hal_handle",

        # External
        "//:gtest",
        ]


gl_linkopts_list = [
    "-lzmq",
    "-lpthread",
    "-pthread",
    "-lz",
    "-lm"
    ]

sdk_copts = ["-Inic/sdk"]
