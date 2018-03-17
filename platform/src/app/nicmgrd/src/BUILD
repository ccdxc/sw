package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license


cc_binary(
        name = "nicmgrd",
        srcs = glob(["*.cc"]),
        copts = [ "-g",
                  "-Inic/include",
                  "-Inic/gen/proto/hal",
                  "-Iplatform/src/lib/nicmgr/include",
                  "-Iplatform/src/lib/misc/include",
                  "-Iplatform/include/common",
                  "-Iplatform/src/lib/cfgspace/include",
                  "-Iplatform/src/lib/pciehost/include",
                  "-Iplatform/src/lib/pciehdevices/include",
                  "-Iplatform/src/lib/pciehw/include",
                  "-Iplatform/src/lib/pcieport/include",
                  "-Iplatform/drivers/linux/eth/ionic",
                ],
        linkopts = [
                     "-lpthread",
                     "-ldl",
                     "-lz",
        ],
        deps = [
                "//platform/src/lib/nicmgr/src:libnicmgr",
                "//platform/src/app/nicmgrd/src/upgrade:libnicmgr_upgrade",
                "//platform/src/app/nicmgrd/src/sysmgr:libnicmgr_sysmgr",
                "//platform/src/app/nicmgrd/src/delphi:libnicmgr_delphi_client",
                "//platform/src/lib/nicmgr/include:libnicmgr_hdrs",
                "//platform/src/lib/misc/include:libmisc_hdrs",
                "//platform/include/common:platformcommon_hdrs",
                "//platform/src/lib/cfgspace/include:libcfgspace_hdrs",
                "//platform/src/lib/pciehost/include:libpciehost_hdrs",
                "//platform/src/lib/pciehdevices/include:libpciehdevices_hdrs",
                "//platform/src/lib/pciehw/include:libpciehw_hdrs",
                "//platform/src/lib/pciehwutil/src:libpciehwutil",
                "//platform/src/lib/pcieport/include:libpcieport_hdrs",
                "//platform/drivers/linux/eth/ionic:liblinuxionic_hdrs",
                "//nic/hal/third-party:grpc",
                "//nic/hal/third-party:libprotobuf",
                "//nic:halproto",
                "//nic:gen_proto_includes",
                "//nic:grpc_includes",
                "//nic:google_includes",
                "//nic/include:base_includes",
                "@sdk//:sdk_headers",
        ],
)

