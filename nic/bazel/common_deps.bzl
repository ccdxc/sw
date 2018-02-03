gl_deps_list = [
        "//nic/hal:hal_src",
        # PI
        "//nic/fte:fte",
        "//nic/hal/periodic",
        "//nic/hal/svc:hal_svc",
        "//nic/utils/trace",
        #"@sdk//obj:sdk_logger",
        "//nic/utils/print",
        "//nic/hal/plugins:plugins",
        "//nic/hal/plugins/proxy:proxyplugin",
        "//nic/utils/host_mem:host_mem",
        #"//nic:gen_proto_includes",
        "//nic/hal/test/utils:haltestutils",
        "//nic/hal/lkl:lkl_api",
        "//nic:lkl",
        "//nic:grpc",
        "//nic:libprotobuf",
        "//nic:halproto",
        #"//nic/hal/pd/control",

        # PD
        "@sdk//obj:sdk_catalog",
        "//nic:asic_libs",
        #"//nic/hal/lib:hal_lib",

        # External
        "//:gtest",
        ]


gl_linkopts_list = [
    "-lzmq",
    "-lpthread",
    "-pthread",
    "-lz",
    "-rdynamic",
    "-lm",
    ]

sdk_copts = ["-Inic/sdk"]
