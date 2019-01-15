#! /usr/bin/python3

import iota.harness.api as api

HOST_NAPLES_DRIVERS_DIR = "%s" % api.HOST_NAPLES_DIR

OS_TYPE_LINUX = "linux"
OS_TYPE_BSD   = "freebsd"

__ionic_modules = [
#    "ionic_rdma",
    "ionic"
]

__linux_directory = "drivers-linux-eth"
__freebsd_directory = "drivers-freebsd-eth"

def RemoveIonicDriverCommands(os_type = OS_TYPE_LINUX):
    cmds = []
    for module in __ionic_modules:
        if os_type == OS_TYPE_LINUX:
            cmds.append("rmmod " + module)
        elif os_type == OS_TYPE_BSD:
            cmds.append("kldunload " + module)
        else:
            assert(0)
    return cmds

def InsertIonicDriverCommands(os_type = OS_TYPE_LINUX, **kwargs):
    driver_args = ' '.join('%s=%r' % x for x in kwargs.items())
    cmds = []
    if os_type == OS_TYPE_LINUX:
        cmds = ["cd %s/%s/ && insmod drivers/eth/ionic/ionic.ko %s" % (HOST_NAPLES_DRIVERS_DIR, __linux_directory, driver_args),
	       ]
    elif os_type == OS_TYPE_BSD:
        for arg  in driver_args.split(" "):
          cmds.append("kenv %s" %  arg)
        cmds.append("kenv xxx_haps=1")
        cmds.append("cd %s/%s/ &&  kldload sys/modules/ionic/ionic.ko"  % (HOST_NAPLES_DRIVERS_DIR, __freebsd_directory))

    return cmds
