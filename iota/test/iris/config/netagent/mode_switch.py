#! /usr/bin/python3
import json
import yaml
import time
import datetime
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common
import iota.test.iris.utils.hal_show as hal_show_utils
import iota.test.iris.config.netagent.api as netagent_api
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
from ipaddress import IPv4Network

def LoadNetworkObjectFromJSON():
    try:
        nw_config = api.GetTopologyDirectory() + "/networks.json"
        with open(nw_config) as f:
            nw_obj = json.load(f)
            for obj in nw_obj['objects']:
                if obj['meta']['name'] == "nw0":
                    return IPv4Network(obj['spec']['ipv4-subnet'])
        return None
    except:
        return None

def LoadNetworkObjectFromYAML():
    try:
        nw_config = api.GetTopologyDirectory() + "/config.yml"
        with open(nw_config) as f:
            nw_obj = yaml.load(f)
            ipam_base = nw_obj['spec']['networks'][0]['network']['ipv4']['ipam_base'].split('/')[0]
            prefix_length = nw_obj['spec']['networks'][0]['network']['ipv4']['prefix_length']
            return IPv4Network(ipam_base + "/" + str(prefix_length))
    except:
        return None

def GetNativeNetworkIpSubnet():
    nw = LoadNetworkObjectFromJSON()
    return  nw if nw else LoadNetworkObjectFromYAML()

def StartSSH():
    api.Logger.info("Starting SSH server on Nodes")
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    enable_sshd = "system enable-sshd"
    copy_key = "update ssh-pub-key -f ~/.ssh/id_rsa.pub"
    for n in api.GetNaplesHostnames():
        #hack for now, need to set date
        cmd = "date -s '{}'".format(datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S"))
        api.Trigger_AddNaplesCommand(req, n, cmd)
        common.AddPenctlCommand(req, n, enable_sshd)
        common.AddPenctlCommand(req, n, copy_key)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)

def DeleteTmpFiles():
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    iota_cmd = "rm -f /data/iota-emulation"
    dev_cmd = "rm -f /sysconfig/config0/device.conf"

    for n in api.GetNaplesHostnames():
        api.Trigger_AddNaplesCommand(req, n, iota_cmd)
        api.Trigger_AddNaplesCommand(req, n, dev_cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)

def restoreBondIp(ip):
    for n in api.GetNaplesHostnames():
        api.SetBondIp(n, str(ip))
        api.Logger.info("%s: Bond IP %s"%(n, str(ip)))
        ip -= 1

def Main(step):
    policy_mode = getattr(step, "policy_Mode", None)
    fwd_mode    = getattr(step, "fwd_mode", None)
    mgmt_intf   = getattr(step, "mgmt_intf", "OOB")
    nw = GetNativeNetworkIpSubnet()
    controller_ip = nw[-2] #Last address in the subnet is broadcast
    mgmt_ip = controller_ip-1
    index = 1

    # Init Netagent
    nodes = api.GetNaplesHostnames()
    netagent_api.Init(nodes, hw = True)

    # Restore the current profile in cfg DB
    if GlobalOptions.skip_setup:
        if mgmt_intf == "IN_BAND":
            restoreBondIp(mgmt_ip)
        return netagent_api.switch_profile(fwd_mode, policy_mode, push=False)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    for n in nodes:
        if mgmt_intf == "OOB":
            ret = common.SetNaplesModeOOB_Static(n, "1.1.1.1", "1.1.1.2/24")
            if ret == None:
                api.Logger.info("Failed to change mode for node: {}".format(n))
                return api.types.status.FAILURE
        elif mgmt_intf == "IN_BAND":
            mgmt_ip_str = "%s/%s"%(mgmt_ip, nw.prefixlen)
            ret = common.SetNaplesModeInband_Static(n, str(controller_ip), mgmt_ip_str)
            if ret == None:
                api.Logger.info("Failed to change mode for node: {}".format(n))
                return api.types.status.FAILURE
            api.SetBondIp(n, str(mgmt_ip))
            mgmt_ip -= 1
        else:
            api.Logger.error("Invalid mgmt interface type : %s"%(mgmt_intf))
            return api.types.status.FAILURE

    api.Logger.info("Switching to profile fwd_mode: %s, policy_mode : %s"%(fwd_mode, policy_mode))
    ret = netagent_api.switch_profile(fwd_mode, policy_mode)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Failed to switch profile to fwd_mode: %s, policy_mode : %s"%(fwd_mode, policy_mode))
        return ret

    # Start SSH server on Naples
    StartSSH()

    # Clean Up temporary files
    DeleteTmpFiles()

    return api.types.status.SUCCESS
