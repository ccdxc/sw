#! /usr/bin/python3
import pdb
import os
import sys
import time
import subprocess
import copy

import iota.harness.api as api
import iota.harness.infra.types as types
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.store as store
import iota.harness.infra.resmgr as resmgr

import iota.protos.pygen.topo_svc_pb2 as topo_pb2
import iota.protos.pygen.types_pb2 as types_pb2

from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
from iota.harness.infra.utils.logger import Logger as Logger

class _Testbed:
    def __init__(self):
        self.curr_ts = None     # Current Testsuite
        self.prev_ts = None     # Previous Testsute
        self.__node_ips = []
        self.__os = None

        self.__fw_upgrade_done = False
        self.__read_testbed_json()
        self.__derive_testbed_attributes()
        return

    def GetTestbedType(self):
        return self.__type

    def GetCurrentTestsuite(self):
        return self.curr_ts

    def GetProvisionUsername(self):
        return self.__tbspec.Provision.Username

    def GetProvisionPassword(self):
        return self.__tbspec.Provision.Password

    def GetOs(self):
        return self.__os

    def __derive_testbed_attributes(self):
        self.__bm_count = 0
        self.__vm_count = 0
        for instance in self.__tbspec.Instances:
            self.__os = getattr(instance, "NodeOs", "linux")
            if instance.Type == "bm":
                self.__bm_count += 1
            elif instance.Type == "vm":
                self.__vm_count += 1
            else:
                assert(0)
        if self.__bm_count:
            if self.__vm_count:
                self.__type = types.tbtype.HYBRID
                Logger.info("Testbed Type = HYBRID")
            else:
                self.__type = types.tbtype.HARDWARE
                Logger.info("Testbed Type = HARDWARE")
        else:
            Logger.info("Testbed Type = SIMULATION")
            self.__type = types.tbtype.SIMULATION
        return

    def __read_testbed_json(self):
        self.__tbspec = parser.JsonParse(GlobalOptions.testbed_json)
        return

    def __get_full_path(self, path):
        if path[0] == '/':
            return path
        return GlobalOptions.topdir + '/' + path

    def __prepare_TestBedMsg(self, ts):
        msg = topo_pb2.TestBedMsg()
        if ts and not GlobalOptions.rerun:
            images = ts.GetImages()
            nap_img = getattr(images, 'naples', None)
            if nap_img:
                msg.naples_image = self.__get_full_path(nap_img)
            ven_img = getattr(images, 'venice', None)
            if ven_img:
                msg.venice_image = self.__get_full_path(ven_img)

        msg.username = self.__tbspec.Provision.Username
        msg.password = self.__tbspec.Provision.Password
        msg.testbed_id = getattr(self, "__tbid", 1)

        for instance in self.__tbspec.Instances:
            node_msg = msg.nodes.add()
            node_msg.type = topo_pb2.TESTBED_NODE_TYPE_SIM
            node_msg.ip_address = instance.NodeMgmtIP
            node_os = getattr(instance, 'NodeOs', None)
            if node_os == "freebsd":
                node_msg.os = topo_pb2.TESTBED_NODE_OS_FREEBSD
            else:
                node_msg.os = topo_pb2.TESTBED_NODE_OS_LINUX
        return msg

    def __cleanup_testbed(self):
        msg = self.__prepare_TestBedMsg(self.prev_ts)
        resp = api.CleanupTestbed(msg)
        if resp is None:
            Logger.error("Failed to cleanup testbed: ")
            return types.status.FAILURE
        return types.status.SUCCESS

    def __cleanup_testbed_script(self):
        logfile = "%s_cleanup.log" % self.curr_ts.Name()
        Logger.info("Cleaning up Testbed, Logfile = %s" % logfile)
        cmd = "timeout 60 ./scripts/cleanup_testbed.py --testbed %s" % GlobalOptions.testbed_json
        if GlobalOptions.rerun:
            cmd = cmd + " --rerun"
        if os.system("%s > %s 2>&1" % (cmd, logfile)) != 0:
            Logger.info("Cleanup testbed failed.")
            return types.status.FAILURE
        return types.status.SUCCESS

    def __get_instance_nic_type(self, instance):
        resource = getattr(instance, "Resource", None)
        if resource is None: return None
        return getattr(resource, "NICType", None)

    def __recover_testbed(self):
        if GlobalOptions.skip_firmware_upgrade or GlobalOptions.dryrun:
            return
        proc_hdls = []
        logfiles = []
        for instance in self.__tbspec.Instances:
            cmd = ["timeout", "1200"]
            if self.__get_instance_nic_type(instance) == "pensando":
                cmd.extend([ "%s/iota/scripts/boot_naples.py" % GlobalOptions.topdir ])
                cmd.extend(["--console-ip", instance.NicConsoleIP])
                cmd.extend(["--console-port", instance.NicConsolePort])
                cmd.extend(["--host-ip", instance.NodeMgmtIP])
                cmd.extend(["--cimc-ip", instance.NodeCimcIP])
                cmd.extend(["--image", "%s/nic/naples_fw.tar" % GlobalOptions.topdir])
                cmd.extend(["--mode", "%s" % api.GetNicMode()])
                cmd.extend(["--drivers-pkg", "%s/platform/gen/drivers-%s.tar.xz" % (GlobalOptions.topdir, instance.NodeOs)])
                cmd.extend(["--uuid", "%s" % instance.Resource.NICUuid])
                cmd.extend(["--os", "%s" % instance.NodeOs])
                if self.__fw_upgrade_done or GlobalOptions.only_reboot:
                    logfile = "%s-%s-reboot.log" % (self.curr_ts.Name(), instance.Name)
                    Logger.info("Rebooting Node %s (logfile = %s)" % (instance.Name, logfile))
                    cmd.extend(["--mode-change"])
                else:
                    logfile = "%s-firmware-upgrade.log" % instance.Name
                    Logger.info("Updating Firmware on %s (logfile = %s)" % (instance.Name, logfile))
            else:
                cmd.extend([ "%s/iota/scripts/reboot_node.py" % GlobalOptions.topdir ])
                cmd.extend(["--host-ip", instance.NodeMgmtIP])
                cmd.extend(["--cimc-ip", instance.NodeCimcIP])
                cmd.extend(["--os", "%s" % instance.NodeOs])
                logfile = "%s-%s-reboot.log" % (self.curr_ts.Name(), instance.Name)
                Logger.info("Rebooting Node %s (logfile = %s)" % (instance.Name, logfile))

            logfiles.append(logfile)
            cmdstring = ""
            for c in cmd: cmdstring += "%s " % c
            Logger.info("Command = ", cmdstring)
            
            loghdl = open(logfile, "w")
            proc_hdl = subprocess.Popen(cmd, stdout=loghdl, stderr=loghdl)
            proc_hdls.append(proc_hdl)

        result = 0
        for idx in range(len(proc_hdls)):
            proc_hdl = proc_hdls[idx]
            while proc_hdl.poll() is None:
                time.sleep(5)
                continue
            if proc_hdl.returncode != 0:
                result = proc_hdl.returncode
                Logger.header("FIRMWARE UPGRADE / MODE CHANGE / REBOOT FAILED: LOGFILE = %s" % logfiles[idx])
                os.system("cat %s" % logfiles[idx])

        if result != 0:
            sys.exit(result)

        self.__fw_upgrade_done = True
        if GlobalOptions.only_firmware_upgrade:
            Logger.info("Stopping after firmware upgrade based on cmdline options.")
            sys.exit(0)
        return


    def __init_testbed(self):
        self.__tbid = getattr(self.__tbspec, 'TestbedID', 1)
        self.__instpool = iter(self.__tbspec.Instances)
        self.__vlan_allocator = resmgr.TestbedVlanAllocator(self.__tbid, api.GetNicMode())
        self.__recover_testbed()
        msg = self.__prepare_TestBedMsg(self.curr_ts)
        resp = api.InitTestbed(msg)
        if resp is None:
            Logger.error("Failed to initialize testbed: ")
            return types.status.FAILURE
        return types.status.SUCCESS

    def InitForTestsuite(self, ts):
        self.prev_ts = self.curr_ts
        self.curr_ts = ts

        if GlobalOptions.dryrun:
            status = types.status.SUCCESS
        else:
            status = self.__cleanup_testbed()
            if status != types.status.SUCCESS:
                return status
            #status = self.__cleanup_testbed_script()
            #if status != types.status.SUCCESS:
            #    return status

        store.Cleanup()
        status = self.__init_testbed()
        return status

    def AllocateInstance(self):
        try:
            inst = next(self.__instpool)
        except:
            Logger.error("No Nodes available in Testbed.")
            sys.exit(1)
        return inst

    def GetDataVlans(self):
        return copy.deepcopy(self.__vlans)

    def AllocateVlan(self):
        return self.__vlan_allocator.Alloc()

__testbed = _Testbed()
store.SetTestbed(__testbed)
