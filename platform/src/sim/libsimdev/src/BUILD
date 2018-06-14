package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_import(
     name = "libsimdev",
     shared_library = "libsimdev.so",
)

cc_binary(
        name = "libsimdev.so",
        srcs = [
                 "simdev.c",
                 "simdev_msg_handler.c",
                 "devparams.c",
                 "generic_dev.c",
                 "eth_dev.c",
                 "accel_dev.c",
                 "virtio_dev.c",
                 "dev_utils.c",
                 "capri_defines.h",
                 "dev_utils.h",
                 "pcie_regs.h",
                 "simdev_impl.h",
                 "virtio_dev_regs.h",
        ],
        copts = [ 
                  "-g",
                  "-Inic/include",
                  "-Iplatform",
        ],
        linkopts = [ 
                  "-Wl,--allow-multiple-definition",
                  "-lpthread",
        ],
        deps = [
                "//platform/src/lib/misc/include:libmisc_hdrs",
                "//platform/src/lib/misc/src:libmisc",
                "//platform/src/lib/intrutils/src:libintrutils",
                "//platform/src/lib/pciehsvc/include:libpciehsvc_hdrs",
                "//platform/src/lib/pciehsvc/src:libpciehsvc",
                "//platform/src/lib/pcieport/src:libpcieport",
                "//platform/src/lib/pcietlp/src:libpcietlp",
                "//platform/src/lib/cfgspace/src:libcfgspace",
                "//platform/src/lib/pal/src:libpal",
                "//platform/src/sim/libsimdev/include:libsimdev_hdrs",
                "//platform/src/sim/libsimlib/include:libsimlib_hdrs",
                "//platform/drivers/linux/eth/ionic:liblinuxionic_hdrs",
                "//nic/include:base_includes",
                "//nic:gen_proto_includes",
        ],
        linkshared = True,
)


