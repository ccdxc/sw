#! /usr/bin/python3
import time
import pdb
import os
import json
from pathlib import Path

import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.harness.infra.utils.parser as parser

OS_TYPE_LINUX = "linux"
OS_TYPE_BSD   = "freebsd"
OS_TYPE_ESX   = "esx"

def LoadLinuxReleaseDriver(tc, node, manifest_file):

    tc.driver_changed = False
    image_manifest = parser.JsonParse(manifest_file)
    driver_images = list(filter(lambda x: x.OS == tc.os, image_manifest.Drivers))[0] 
    if driver_images is None: 
        api.Logger.error("Unable to load image manifest") 
        return api.types.status.FAILURE

    drImgFile = os.path.join("/sw", driver_images.drivers_pkg)
    api.Logger.info("Fullpath for driver image: " + drImgFile)
    resp = api.CopyToHost(node, [drImgFile], "")
    if not api.IsApiResponseOk(resp):
        api.Logger.error("Failed to copy %s" % drImgFile)
        return api.types.status.FAILURE

    rundir = os.path.basename(driver_images.drivers_pkg).split('.')[0]
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(req, node, "tar -xf " + os.path.basename(driver_images.drivers_pkg))
    api.Trigger_AddHostCommand(req, node, "./build.sh", rundir=rundir)

    resp = api.Trigger(req)

    if not api.IsApiResponseOk(resp):
        api.Logger.error("TriggerCommand for driver build failed")
        return api.types.status.FAILURE

    for cmd in resp.commands:
        if cmd.exit_code != 0 and cmd.command != './build.sh':  # Build.sh could fail -ignored (FIXME)
            api.Logger.error("Failed to exec cmds to build/load new driver")
            return api.types.status.FAILURE

    api.Logger.info("New driver image is built on target host. Prepare to load")

    if host.UnloadDriver(tc.os, node) != api.types.status.SUCCESS:
        api.Logger.error("Failed to unload current driver - proceeding")
        tc.driver_changed = True

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if tc.os == OS_TYPE_LINUX: 
        api.Trigger_AddHostCommand(req, node, "insmod " + 
                os.path.join(rundir, "drivers/eth/ionic/ionic.ko"))
    elif tc.os == OS_TYPE_BSD: 
        api.Trigger_AddHostCommand(req, node, "kldload " + 
                os.path.join(rundir, "drivers/eth/ionic/ionic.ko"))
    resp = api.Trigger(req)

    # api.RestartNodes(tc.nodes)
    tc.driver_changed = True

    if not api.IsApiResponseOk(resp):
        api.Logger.error("TriggerCommand for driver installation failed")
        return api.types.status.FAILURE

    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error("Failed to exec cmds to build/load new driver")
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def GetLinuxFwDriverVersion(node):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "/naples/nodeinit.sh --version"
    api.Logger.info("Collect software version with : %s" % cmd)
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)

    if not api.Trigger_IsSuccess(resp): 
        api.Logger.error("Failed to collect version") 
        return None, None

    # Parse resp stdout for following
    # Example:
    # version: 1.3.0-E-123-13-g827ffc8
    # firmware-version: 1.1.1-E-15
    ethtool_lines = resp.commands[0].stdout.split('\n')
    fw_version = list(filter(lambda x: 'firmware-version' in x, ethtool_lines))[0].rstrip().split()[1]
    dr_version = list(filter(lambda x: 'version' in x, ethtool_lines))[0].rstrip().split()[1]
    return fw_version, dr_version

def LinuxReInitMgmtIP(node):
    # Run nodeinit.sh to restore host->mgmt communication
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "/naples/nodeinit.sh --mgmt_only --own_ip %s --trg_ip %s" % (api.GetPrimaryIntNicMgmtIpNext(), api.GetPrimaryIntNicMgmtIp())
    api.Logger.info("Resetting mgmt-if with : %s" % cmd)
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)

    if not api.Trigger_IsSuccess(resp): 
        api.Logger.error("nodeinit.sh to change mgmt ip cmd failed") 
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def ESXiReInitMgmtIP(node):
    # Run penctl.sh to restore host->mgmt communication
    return api.types.status.SUCCESS

def LoadFirmware(tc, node, manifest_file):

    image_manifest = parser.JsonParse(manifest_file)
    fw_images = list(filter(lambda x: x.naples_type == "capri", image_manifest.Firmwares))[0] 
    if fw_images is None: 
        api.Logger.error("Unable to load image manifest") 
        return api.types.status.FAILURE

    tc.fw_changed = False
    api.Logger.info("Fullpath for firmware image to load: %s " % fw_images.image)

    if tc.os == OS_TYPE_LINUX:
        fw_version, _ = GetLinuxFwDriverVersion(node)
    else:
        fw_version = None

    fwImgFile = os.path.join("/sw", fw_images.image)
    if fw_version == '1.1.1-E-15':
        # Naples with Fw 1.1.1-E-15 has no OOB and IntMgmt Ip is fixed 
        resp = api.CopyToNaples(node, [fwImgFile], image_manifest.Version, 
                                via_oob=False, naples_dir="data", 
                                nic_mgmt_ip=api.GetPrimaryIntNicMgmtIpNext())
        if not api.IsApiResponseOk(resp):
            api.Logger.info("Failed to copy naples_fw.tar with via_oob=True")
            return api.types.status.FAILURE

        for cmd in resp.commands:
            if cmd.exit_code != 0:
                api.Logger.error("Failed to copy %s naples_fw.tar via_oob=True" % image_manifest.Version) 
                return api.types.status.FAILURE
    else:
        resp = api.CopyToNaples(node, [fwImgFile], image_manifest.Version, via_oob=True, naples_dir="data")
        if not api.IsApiResponseOk(resp):
            api.Logger.info("Failed to copy naples_fw.tar with via_oob=True")
            return api.types.status.FAILURE

        for cmd in resp.commands:
            if cmd.exit_code != 0:
                api.Logger.error("Failed to copy %s naples_fw.tar via_oob=True" % image_manifest.Version)

                # Try with via_oob=False
                resp = api.CopyToNaples(node, [fwImgFile], image_manifest.Version, via_oob=False, naples_dir="data")
                if not api.Trigger_IsSuccess(resp): 
                    api.Logger.error("Failed to copy naples_fw.tar to target naples") 
                    return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(req, node, "sync")
    api.Trigger_AddNaplesCommand(req, node, "/nic/tools/sysupdate.sh -p /data/naples_fw.tar", timeout=120)

    resp = api.Trigger(req)
    if not api.Trigger_IsSuccess(resp): 
        api.Logger.error("sysupdate.sh cmd failed") 
        return api.types.status.FAILURE

    tc.fw_changed = True

    api.RestartNodes([node])

    if tc.os == OS_TYPE_LINUX: 
        return LinuxReInitMgmtIP(node)
    elif tc.os == OS_TYPE_ESX:
        return ESXiReInitMgmtIP(node)

    return api.types.status.SUCCESS

def LoadESXReleaseDriver(tc, node, manifest_file):
    image_manifest = parser.JsonParse(manifest_file)
    driver_images = list(filter(lambda x: x.OS == tc.os, image_manifest.Drivers))[0] 
    if driver_images is None: 
        api.Logger.error("Unable to load image manifest") 
        return api.types.status.FAILURE

    drImgFile = os.path.join("/sw", driver_images.drivers_pkg)
    if driver_images.pkg_file_type == "SrcBundle":
        # Copy and extract the file and find ionic*.vib file to load
        os.system("tar -xf " + drImgFile)
        pkgdir = os.path.basename(driver_images.drivers_pkg).split('.')[0]
        plist = list(Path(pkgdir).rglob('*.vib'))
        if not plist:
            api.Logger.error("Unable to find VIB file in driver-pkg: %s" % driver_images.drivers_pkg)
            return api.types.status.FAILURE
        vib_file = str(plist[0].absolute())
    elif driver_images.pkg_file_type == "VIB":
        vib_file = drImgFile
    else:
        api.Logger.error("Unknown format for driver-pkg: %s - aborting" % driver_images.drivers_pkg)
        return api.types.status.FAILURE

    api.Logger.info("Loading %s on node: %s" % (vib_file, node))
    resp = api.CopyToEsx(node, [vib_file], host_dir="")
    if not api.IsApiResponseOk(resp):
        api.Logger.error("Failed to copy %s", driver_images.drivers_pkg)
        return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(req, node, 
            "sshpass -p %s ssh -o  UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no %s@%s esxcli software vib install -v=/tmp/%s -f" % 
            (api.GetTestbedEsxPassword(), api.GetTestbedEsxUsername(), api.GetEsxHostIpAddress(node), os.path.basename(vib_file)))

    resp = api.Trigger(req)
    if not api.Trigger_IsSuccess(resp): 
        api.Logger.error("Failed to install driver") 
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

