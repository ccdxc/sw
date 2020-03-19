#! /usr/bin/python3
import copy
import os
import json
import pdb
import socket
import subprocess
import sys
import time
import traceback

import iota.harness.api as api
import iota.harness.infra.types as types
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.store as store
import iota.harness.infra.resmgr as resmgr
import iota.harness.infra.getNicHostInfo as gnhi

import iota.protos.pygen.topo_svc_pb2 as topo_pb2
import iota.protos.pygen.iota_types_pb2 as types_pb2

from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
from iota.harness.infra.utils.logger import Logger as Logger
from iota.harness.infra.exceptions import *

ESX_CTRL_VM_BRINGUP_SCRIPT = "%s/iota/bin/iota_esx_setup" % (GlobalOptions.topdir)

def _get_driver_version(file):
    return os.path.basename(os.path.dirname(os.path.realpath(file)))

def setUpSwitchQos(switch_ctx):
    #Needed for RDMA, neeed to be set from testsuite.
    switch_ctx.flow_control_receive = True
    switch_ctx.flow_control_send = True
    switch_ctx.mtu = 9216
    switch_ctx.qos.name = "pausenq"
    qosClass = switch_ctx.qos.qos_classes.add()
    qosClass.name = "c-nq3"
    qosClass.mtu = 1500
    qosClass.pause_pfc_cos = 3
    qosClass = switch_ctx.qos.qos_classes.add()
    qosClass.name = "c-nq2"
    qosClass.mtu = 1500
    qosClass.pause_pfc_cos = 100  # set cos to an invalid value
    qosClass = switch_ctx.qos.qos_classes.add()
    qosClass.name = "c-nq1"
    qosClass.mtu = 1500
    qosClass.pause_pfc_cos = 100  # set cos to an invalid value
    qosClass = switch_ctx.qos.qos_classes.add()
    qosClass.name = "c-nq-default"
    qosClass.mtu = 9216
    qosClass.pause_pfc_cos = 0

def updateMultiNicInfo():
    try:
        nicInfo = gnhi.getNicHostInfo(GlobalOptions.testbed_json)
        nicInfo = nicInfo['Instances']
        if len(nicInfo) == 0:
            Logger.debug('no instance found in testbed topology file {0}'.format(GlobalOptions.testbed_json))
        elif len(nicInfo[0]['Nics']) == 0:
            Logger.debug('no nics found for instance 0 in testbed topology file {0}'.format(GlobalOptions.testbed_json))
        elif len(nicInfo[0]['Nics'][0]['Ports']) == 0:
            Logger.debug('no ports found for multi nic 0 in testbed topology file {0}'.format(GlobalOptions.testbed_json))
        else:
            ip = nicInfo[0]['Nics'][0]['Ports'][0].get('IP',None) 
            if not ip:
                Logger.debug('no ip found for multi nic 0 / port 0 in testbed topology file {0}'.format(GlobalOptions.testbed_json))
            else:
                msg = 'determined multi nic mgmt ip to be: {0}'.format(ip)
                print(msg)
                Logger.info(msg)
                store.SetPrimaryIntNicMgmtIp(ip)
    except:
        Logger.debug('failed to determine internal mnic ip. error was: {0}'.format(traceback.format_exc()))
        Logger.warn('setting nic int mgmt ip to default of 169.254.0.1')
        store.SetPrimaryIntNicMgmtIp('169.254.0.1')

class _Testbed:

    SUPPORTED_OS = ["linux", "freebsd", "esx"]

    def __init__(self):
        self.curr_ts = None     # Current Testsuite
        self.prev_ts = None     # Previous Testsute
        self.image_manifest_file = os.path.join(GlobalOptions.topdir, "images", "latest.json")
        self.__node_ips = []
        self.__os = set()
        self.esx_ctrl_vm_ip = None

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

    def GetProvisionEsxUsername(self):
        return self.__tbspec.Provision.Vars.EsxUsername

    def GetProvisionEsxPassword(self):
        return self.__tbspec.Provision.Vars.EsxPassword

    def GetOs(self):
        return self.__os

    def IsSimulation(self):
        return self.__type == types.tbtype.SIMULATION

    def __derive_testbed_attributes(self):
        self.__bm_count = 0
        self.__vm_count = 0
        for instance in self.__tbspec.Instances:
            if instance.Type == "bm":
                self.__bm_count += 1
                #if self.__os:
                #    assert(self.__os == getattr(instance, "NodeOs", "linux"))
                self.__os.add(getattr(instance, "NodeOs", "linux"))
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
        if not self.__tbspec.Instances:
            msg = 'failed to process testbed file {0}. no instances found'.format(GlobalOptions.testbed_json)
            print(msg)
            Logger.debug(msg)
            sys.exit(types.status.OFFLINE_TESTBED)
        updateMultiNicInfo()
        try:
            for instance in self.__tbspec.Instances:
                if hasattr(self.__tbspec.Provision, "Vars") and hasattr(self.__tbspec.Provision.Vars, 'BmOs') and instance.Type == "bm":
                    instance.NodeOs = self.__tbspec.Provision.Vars.BmOs
                if hasattr(self.__tbspec.Provision, "Vars") and hasattr(self.__tbspec.Provision.Vars, 'VmOs') and instance.Type == "vm":
                    instance.NodeOs = self.__tbspec.Provision.Vars.VmOs
            return
        except:
            print('failed parsing testbed json')
            Logger.debug("failed parsing testbed json. error was: {0}".format(traceback.format_exc()))
            Logger.debug("failed on node instance: {0}".format(instance.__dict__)) 
            sys.exit(types.status.OFFLINE_TESTBED)

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
            nap_sim_img = getattr(images, 'naples_sim', None)
            if nap_sim_img:
                msg.naples_sim_image = self.__get_full_path(nap_sim_img)


        msg.username = self.__tbspec.Provision.Username
        msg.password = self.__tbspec.Provision.Password
        msg.testbed_id = getattr(self, "__tbid", 1)
        msg.native_vlan = self.GetNativeVlan()
        Logger.info("Native Vlan %s" % str(msg.native_vlan))

        is_vcenter = False
        for instance in self.__tbspec.Instances:
            node_msg = msg.nodes.add()
            node_msg.type = topo_pb2.TESTBED_NODE_TYPE_SIM
            node_msg.ip_address = instance.NodeMgmtIP
            node_os = getattr(instance, 'NodeOs', 'linux')
            if node_os == "freebsd":
                node_msg.os = topo_pb2.TESTBED_NODE_OS_FREEBSD
            elif node_os == "esx":
                node_msg.os = topo_pb2.TESTBED_NODE_OS_ESX
                node_msg.esx_username = self.__tbspec.Provision.Vars.EsxUsername
                node_msg.esx_password = self.__tbspec.Provision.Vars.EsxPassword
            else:
                node_msg.os = topo_pb2.TESTBED_NODE_OS_LINUX

            if getattr(instance, "Tag", None) ==  "vcenter":
                node_msg.os = topo_pb2.TESTBED_NODE_OS_VCENTER
                node_msg.dc_name = GlobalOptions.dc_name
                node_msg.switch = GlobalOptions.distributed_switch
                node_msg.esx_username = self.__tbspec.Provision.Vars.VcenterUsername
                node_msg.esx_password = self.__tbspec.Provision.Vars.VcenterPassword
                node_msg.license = self.__tbspec.Provision.Vars.VcenterLicense
                is_vcenter = True

            if is_vcenter:
                license = msg.licenses.add()
                license.username = self.__tbspec.Provision.Vars.VcenterUsername
                license.password =  self.__tbspec.Provision.Vars.VcenterPassword
                license.key = self.__tbspec.Provision.Vars.VcenterLicense
                license.type = topo_pb2.License.Type.Name(topo_pb2.License.LICENSE_VCENTER) 




            res = instance.Resource
            if getattr(res,"ApcIP",None):
                node_msg.apc_info.ip = res.ApcIP
                node_msg.apc_info.port = res.ApcPort
                node_msg.apc_info.username = res.ApcUsername
                node_msg.apc_info.password = res.ApcPassword
            if getattr(instance, "NodeCimcIP", "") != "":
                node_msg.cimc_ip_address = instance.NodeCimcIP
                node_msg.cimc_username = getattr(instance,"NodeCimcUsername","admin")
                node_msg.cimc_password = getattr(instance,"NodeCimcPassword","N0isystem$")
            if getattr(res, "NodeCimcNcsiIP", "") != "":
                node_msg.cimc_ncsi_ip = res.NodeCimcNcsiIP

            #If Vlan base not set, ask topo server to allocate.
            if not (getattr(self.__tbspec, "TestbedVlanBase", None) or GlobalOptions.skip_switch_init):
                switch_ips = {}
                if instance.Type == "bm":
                    for nw in instance.DataNetworks:
                        switch_ctx = switch_ips.get(nw.SwitchIP, None)
                        if not switch_ctx:
                            switch_ctx = msg.data_switches.add()
                            switch_ips[nw.SwitchIP] = switch_ctx

                            switch_ctx.username = nw.SwitchUsername
                            switch_ctx.password = nw.SwitchPassword
                            switch_ctx.ip = nw.SwitchIP
                            # This should from testsuite eventually or each testcase should be able to set
                            switch_ctx.speed = topo_pb2.DataSwitch.Speed_auto
                            # igmp disabled for now
                            switch_ctx.igmp_disabled = True
                            setUpSwitchQos(switch_ctx)

                        switch_ctx.ports.append(nw.Name)

                    #Testbed ID is the last one.
                    msg.testbed_id = getattr(instance, "ID", 0)
                    Logger.info("Testbed ID used %s" % str(msg.testbed_id))
            else:
                if not GlobalOptions.skip_setup and not GlobalOptions.skip_switch_init:
                    Logger.info ("Setting up Testbed Network")
                    resp = self.SetupTestBedNetwork()
                    if resp != types.status.SUCCESS:
                        Logger.info("Vlan programming failed, ignoring")
                        #assert(0)
                else:
                    Logger.info ("Skipped switch setup")
        return msg

    def __cleanup_testbed(self):
        msg = self.__prepare_TestBedMsg(self.prev_ts)
        resp = api.CleanupTestbed(msg)
        if resp is None:
            Logger.error("Failed to cleanup testbed: ")
            return types.status.FAILURE
        return types.status.SUCCESS

    def __cleanup_testbed_script(self):
        logfile = "%s/%s_cleanup.log" % (GlobalOptions.logdir, self.curr_ts.Name())
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
        if resource is None: return "pensando-sim"
        return getattr(resource, "NICType", "pensando-sim")

    def __recover_testbed(self, manifest_file, **kwargs):
        if GlobalOptions.dryrun or GlobalOptions.skip_setup:
            return
        proc_hdls = []
        logfiles = []
        logfile  = ''
        naples_host_only = kwargs.get('naples_host_only', False) 
        firmware_reimage_only = kwargs.get('firmware_reimage_only', False)
        driver_reimage_only = kwargs.get('driver_reimage_only', False)

        for instance in self.__tbspec.Instances:
            cmd = ["timeout", "2400"]

            if getattr(instance, "NicIntMgmtIP",'') == '':
                instance.NicIntMgmtIP = api.GetPrimaryIntNicMgmtIp()
            else:
                instance.NicIntMgmtIP = getattr(instance, "NicIntMgmtIP")
            if not hasattr(instance, "NicMgmtIP") or instance.NicMgmtIP is None or instance.NicMgmtIP.replace(" ", "") == '':
                instance.NicMgmtIP = instance.NicIntMgmtIP
            if self.__get_instance_nic_type(instance) in ["pensando", "naples"]:
                #if instance.NodeOs == "esx":
                #    cmd.extend([ "%s/iota/scripts/boot_naples.py" % GlobalOptions.topdir ])
                #else:

                cmd.extend([ "%s/iota/scripts/boot_naples_v2.py" % GlobalOptions.topdir ])

                if self.curr_ts.GetNicMode() == "bitw":
                    if (instance.NicMgmtIP == "" or instance.NicMgmtIP == None):
                        Logger.error("Nic Management IP not specified for : %s, mandatory for bump in wire mode" % instance.NodeMgmtIP)
                        sys.exit(1)
                    else:
                        cmd.extend(["--naples-only-setup"])

                    mem_size = None
                    if GlobalOptions.pipeline in [ "iris", "apollo", "artemis", "apulu" ]:
                        mem_size = "8g"
                    mem_size = getattr(instance, "NicMemorySize", mem_size)
                    if mem_size is not None:
                        cmd.extend(["--naples-mem-size", mem_size])

                if firmware_reimage_only: 
                    cmd.extend(["--naples-only-setup"])
                elif driver_reimage_only:
                    cmd.extend(["--only-init"])

                # XXX workaround: remove when host mgmt interface works for apulu
                if GlobalOptions.pipeline in [ "apulu" ]:
                    cmd.extend(["--no-mgmt"])

                cmd.extend(["--testbed", GlobalOptions.testbed_json])
                cmd.extend(["--instance-name", instance.Name])
                cmd.extend(["--naples", GlobalOptions.naples_type])
                cmd.extend(["--mnic-ip", instance.NicIntMgmtIP])
                nics = getattr(instance, "Nics", None)
                if nics != None and len(nics) != 0:
                    for nic in nics:
                        for port in getattr(nic, "Ports", []):
                            cmd.extend(["--mac-hint", port.MAC])
                            break
                cmd.extend(["--mode", "%s" % api.GetNicMode()])
                if instance.NodeOs == "esx":
                    pass
                    # cmd.extend(["--esx-script", ESX_CTRL_VM_BRINGUP_SCRIPT])
                if GlobalOptions.skip_driver_install:
                    cmd.extend(["--skip-driver-install"]) 
                if GlobalOptions.use_gold_firmware: 
                    cmd.extend(["--use-gold-firmware"]) 
                if GlobalOptions.fast_upgrade: 
                    cmd.extend(["--fast-upgrade"]) 
                cmd.extend(["--uuid", "%s" % instance.Resource.NICUuid])
                cmd.extend(["--image-manifest", manifest_file])

                if self.__fw_upgrade_done or GlobalOptions.only_reboot:
                    logfile = "%s/%s-%s-reboot.log" % (GlobalOptions.logdir, self.curr_ts.Name(), instance.Name)
                    Logger.info("Rebooting Node %s (logfile = %s)" % (instance.Name, logfile))
                    cmd.extend(["--only-mode-change"])
                elif GlobalOptions.skip_firmware_upgrade:
                    logfile = "%s/%s-%s-reinit.log" % (GlobalOptions.logdir, self.curr_ts.Name(), instance.Name)
                    Logger.info("Reiniting Node %s (logfile = %s)" % (instance.Name, logfile))
                    cmd.extend(["--only-init"])
                else:
                    logfile = "%s/%s-firmware-upgrade.log" % (GlobalOptions.logdir, instance.Name)
                    Logger.info("Updating Firmware on %s (logfile = %s)" % (instance.Name, logfile))
                if GlobalOptions.netagent: 
                    cmd.extend(["--auto-discover-on-install"]) 
            else:
                if GlobalOptions.skip_firmware_upgrade or instance.Type == "vm" or naples_host_only:
                    continue 
                cmd.extend([ "%s/iota/scripts/reboot_node.py" % GlobalOptions.topdir ])
                cmd.extend(["--host-ip", instance.NodeMgmtIP])
                cmd.extend(["--cimc-ip", instance.NodeCimcIP])
                if hasattr(instance, "NodeCimcUsername"):
                    cmd.extend(["--cimc-username", instance.NodeCimcUsername])
                cmd.extend(["--os", "%s" % instance.NodeOs])
                if instance.NodeOs == "esx": 
                    cmd.extend(["--host-username", self.__tbspec.Provision.Vars.EsxUsername]) 
                    cmd.extend(["--host-password", self.__tbspec.Provision.Vars.EsxPassword]) 
                else: 
                    cmd.extend(["--host-username", self.__tbspec.Provision.Username]) 
                    cmd.extend(["--host-password", self.__tbspec.Provision.Password])

                logfile = "%s/%s-%s-reboot.log" % (GlobalOptions.logdir, self.curr_ts.Name(), instance.Name)
                Logger.info("Rebooting Node %s (logfile = %s)" % (instance.Name, logfile))

            if (logfile):
                logfiles.append(logfile)
                cmdstring = ""
                for c in cmd: cmdstring += "%s " % c
                Logger.info("Command = ", cmdstring)
                loghdl = open(logfile, "w")
                proc_hdl = subprocess.Popen(cmd, stdout=loghdl, stderr=subprocess.PIPE)
                proc_hdls.append(proc_hdl)

        result = 0
        try:
            for idx in range(len(proc_hdls)):
                proc_hdl = proc_hdls[idx]
                Logger.debug('Firmware upgrade started at time: {0}'.format(time.asctime()))
                while proc_hdl.poll() is None:
                    time.sleep(5)
                    continue
                if proc_hdl.returncode != 0:
                    result = proc_hdl.returncode
                    _, err = proc_hdl.communicate()
                    Logger.header("FIRMWARE UPGRADE / MODE CHANGE / REBOOT FAILED: LOGFILE = %s" % logfiles[idx])
                    Logger.error("Firmware upgrade failed : " + err.decode())
                else:
                    Logger.debug('Firmware upgrade finished at time: {0}'.format(time.asctime()))
        except KeyboardInterrupt:
            result=2
            err="SIGINT detected. terminating boot_naples_v2 scripts" 
            Logger.debug(err)
            for proc in proc_hdls:
                Logger.debug("sending SIGKILL to pid {0}".format(proc.pid))
                proc.terminate()

        if result != 0:
            sys.exit(result)

        self.__fw_upgrade_done = True
        if GlobalOptions.only_firmware_upgrade:
            Logger.info("Stopping after firmware upgrade based on cmdline options.")
            sys.exit(0)

        #read multi nic info again in case ip changed after running boot naples
        updateMultiNicInfo()
        return

    def __init_testbed(self):
        self.__tbid = getattr(self.__tbspec, 'TestbedID', 1)
        self.__vlan_base = getattr(self.__tbspec, 'TestbedVlanBase', 1)
        self.__vlan_allocator = resmgr.TestbedVlanAllocator(self.__vlan_base, api.GetNicMode())
        resp = None
        msg = self.__prepare_TestBedMsg(self.curr_ts)
        if not GlobalOptions.skip_setup:
            try:
                self.__recover_testbed(self.image_manifest_file)
            except:
                Logger.error("Failed to recover testbed")
                Logger.debug(traceback.format_exc())
                return types.status.CRITICAL
            if GlobalOptions.dryrun:
                status = types.status.SUCCESS
            else:
                status = self.__cleanup_testbed()
                if status != types.status.SUCCESS:
                    return status
                #status = self.__cleanup_testbed_script()
                #if status != types.status.SUCCESS:
                #    return status

            resp = api.InitTestbed(msg)
        else:
            resp = api.GetTestbed(msg)
        if resp is None:
            Logger.error("Failed to initialize testbed: ")
            raise OfflineTestbedException
            #return types.status.FAILURE
        if not api.IsApiResponseOk(resp):
            Logger.error("Failed to initialize testbed: ")
            raise OfflineTestbedException
            #return types.status.FAILURE
        for instance,node in zip(self.__tbspec.Instances, resp.nodes):
            if getattr(instance, 'NodeOs', None) == "esx":
                instance.esx_ctrl_vm_ip = node.esx_ctrl_node_ip_address
        Logger.info("Testbed allocated vlans {}".format(resp.allocated_vlans))
        if resp.allocated_vlans:
            tbvlans = []
            for vlan in resp.allocated_vlans:
                tbvlans.append(vlan)
            self.__vlan_allocator = resmgr.TestbedVlanManager(tbvlans)
        self.__instpool = copy.deepcopy(self.__tbspec.Instances)


        return types.status.SUCCESS

    def ReImageTestbed(self, req_json):
        """
        Build a new image-manifest file for initializing Naples and Driver
        """
        # Generate new image manifest file
        Logger.info("Building new image-manifest {}".format(req_json))

        reimg_req = parser.ParseJsonStream(req_json)

        with open(self.image_manifest_file, "r") as fh:
            new_img_manifest = json.loads(fh.read())
        manifest_file = self.image_manifest_file
        self.__fw_upgrade_done = False
        reimage_driver = getattr(reimg_req, 'InstallDriver', False)
        reimage_firmware = getattr(reimg_req, 'InstallFirmware', False)

        # pick the non-latest versions
        if reimage_driver:
            # driver image to be changed
            new_img_manifest["Drivers"] = None
            dr_img_manifest_file = os.path.join(GlobalOptions.topdir, 
                                                "images", reimg_req.DriverVersion + ".json")
            with open(dr_img_manifest_file, "r") as fh:
                dr_img_manifest = json.loads(fh.read())
            new_img_manifest["Drivers"] = dr_img_manifest["Drivers"]

        if reimage_firmware:
            # Firmware image to be changed
            new_img_manifest["Firmwares"] = None
            fw_img_manifest_file = os.path.join(GlobalOptions.topdir, 
                                                "images", reimg_req.FirmwareVersion + ".json")
            with open(fw_img_manifest_file, "r") as fh:
                fw_img_manifest = json.loads(fh.read())
            new_img_manifest["Firmwares"] = fw_img_manifest["Firmwares"]

        if reimage_driver or reimage_firmware:
            new_img_manifest["Version"] = "NA" # TODO

            # Create {GlobalOptions.topdir}/images if not exists
            folder = os.path.join(GlobalOptions.topdir, "images")
            if not os.path.isdir(folder):
                os.mkdir(folder)
            manifest_file = os.path.join(folder, reimg_req.TestCaseName + ".json")
            with open(manifest_file, "w") as fh:
                fh.write(json.dumps(new_img_manifest, indent=2))

            # Call API to reimage testbed : restrict for naples nodes only
            GlobalOptions.only_reboot = False
            GlobalOptions.skip_firmware_upgrade = False
            self.__recover_testbed(manifest_file, 
                                   driver_reimage_only=reimage_driver and not reimage_firmware, 
                                   firmware_reimage_only=reimage_firmware and not reimage_driver, 
                                   naples_host_only=True)
        return types.status.SUCCESS

    def InitForTestsuite(self, ts=None):
        if ts is not None:
            self.prev_ts = self.curr_ts
            self.curr_ts = ts

        store.Cleanup()
        status = self.__init_testbed()
        return status

    def AllocateInstance(self, type, tag=None):
        for instance in self.__instpool:
            if instance.Type == type:
                if tag != None and tag != getattr(instance, "Tag", None):
                    continue
                self.__instpool.remove(instance)
                return instance
        else:
            Logger.error("No Nodes available in Testbed of type : %s" % type)
            sys.exit(1)

    def GetProvisionParams(self):
        return getattr(self.__tbspec.Provision, "Vars", None)

    def GetVlans(self):
        return self.__vlan_allocator.Vlans()

    def SetVlans(self, vlans):
        self.__vlan_allocator = resmgr.TestbedVlanManager(vlans)

    def AllocateVlan(self):
        return self.__vlan_allocator.Alloc()

    def ResetVlanAlloc(self):
        self.__vlan_allocator.Reset()

    def GetVlanCount(self):
        return self.__vlan_allocator.Count()

    def GetVlanBase(self):
        return self.__vlan_base

    def GetVlanRange(self):
        return self.__vlan_allocator.VlanRange()

    def GetNativeVlan(self):
        vlan = 0
        if getattr(self.__tbspec, "TestbedVlanBase", None):
            vlan = self.__vlan_allocator.VlanNative()
        else:
            nw = getattr(self.__tbspec, "Network", None)
            if nw:
                vlan = getattr(nw, "VlanID", 0)
        return vlan

    def UnsetVlansOnTestBed(self):
        #First Unset the Switch
        unsetMsg = topo_pb2.SwitchMsg()
        unsetMsg.op = topo_pb2.VLAN_CONFIG
        for instance in self.__tbspec.Instances:
            switch_ips = {}
            if instance.Type == "bm":
                for nw in instance.DataNetworks:
                    switch_ctx = switch_ips.get(nw.SwitchIP, None)
                    if not switch_ctx:
                        switch_ctx = unsetMsg.data_switches.add()
                        switch_ips[nw.SwitchIP] = switch_ctx
                    switch_ctx.username = nw.SwitchUsername
                    switch_ctx.password = nw.SwitchPassword
                    switch_ctx.ip = nw.SwitchIP
                    switch_ctx.ports.append(nw.Name)
      
        vlans = self.GetVlanRange()
        for ip, switch in switch_ips.items():
             unsetMsg.vlan_config.unset = True
             unsetMsg.vlan_config.vlan_range = vlans
             unsetMsg.vlan_config.native_vlan = self.GetNativeVlan()

        resp = api.DoSwitchOperation(unsetMsg)
        if not api.IsApiResponseOk(resp):
            return types.status.FAILURE
        return types.status.SUCCESS

    def SetupTestBedNetwork(self):
        #First Unset the Switch
        setMsg = topo_pb2.SwitchMsg()
        setMsg.op = topo_pb2.VLAN_CONFIG
        switch_ips = {}
        for instance in self.__tbspec.Instances:
            if instance.Type == "bm":
                for nw in instance.DataNetworks:
                    switch_ctx = switch_ips.get(nw.SwitchIP, None)
                    if not switch_ctx:
                        switch_ctx = setMsg.data_switches.add()
                        switch_ips[nw.SwitchIP] = switch_ctx
                    switch_ctx.username = nw.SwitchUsername
                    switch_ctx.password = nw.SwitchPassword
                    switch_ctx.ip = nw.SwitchIP
                    switch_ctx.ports.append(nw.Name)
      
        vlans = self.GetVlanRange()
        for ip, switch in switch_ips.items():
             setMsg.vlan_config.unset = False
             setMsg.vlan_config.vlan_range = vlans
             setMsg.vlan_config.native_vlan = self.GetNativeVlan()

        resp = api.DoSwitchOperation(setMsg)
        if not api.IsApiResponseOk(resp):
            return types.status.FAILURE
        return types.status.SUCCESS

    def SetUpTestBedInHostToHostNetworkMode(self):
        resp = self.UnsetVlansOnTestBed()
        if resp != types.status.SUCCESS:
            return resp

        msgs = []
        switches = []
        native_vlans = []
        self.ResetVlanAlloc()
        for instance in self.__tbspec.Instances:
            if instance.Type == "bm":
                for index, nw in enumerate(instance.DataNetworks):
                    if len(switches) < index + 1:
                        msg = topo_pb2.SwitchMsg()
                        msg.op = topo_pb2.VLAN_CONFIG
                        switch_ctx = msg.data_switches.add()
                        switch_ctx.username = nw.SwitchUsername
                        switch_ctx.password = nw.SwitchPassword
                        switch_ctx.ip = nw.SwitchIP
                        native_vlans.append(self.AllocateVlan())
                        msgs.append(msg)
                        msg.vlan_config.unset = False
                        msg.vlan_config.vlans.append(native_vlans[index])
                        msg.vlan_config.native_vlan = native_vlans[index]
                    else:
                        switch_ctx = switches[index]
                    switch_ctx.ports.append(nw.Name)

        for msg in msgs:
            resp = api.DoSwitchOperation(msg)
            if not api.IsApiResponseOk(resp):
               return types.status.FAILURE
        return types.status.SUCCESS

__testbed = _Testbed()
store.SetTestbed(__testbed)
