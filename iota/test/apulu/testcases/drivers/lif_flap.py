#!/usr/bin/python3
import yaml

import iota.harness.api as api
import iota.test.apulu.utils.pdsctl as pdsctl
import types_pb2 as types_pb2

from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

# utilities from dol infra
import apollo.config.utils as yaml_utils

# Returns output of pdsctl show lif
def GetLifInfo(node, if_name=None):
    if GlobalOptions.dryrun:
        return 1, ''
    retval, output = pdsctl.ExecutePdsctlShowCommand(node, 'lif', yaml=True)

    if not retval:
        api.Logger.error("Failed to get lif info from pdsctl")
        output = ''
        return retval, output

    if if_name:
        # split output and fetch the info for given if_name
        lif_entries = output.split("---")
        for lif in lif_entries:
            yamlOp = yaml.load(lif, Loader=yaml.FullLoader)
            if not yamlOp:
                continue
            #api.Logger.info("LIF Yaml Output for ifname:%s, output:%s"%(if_name, yamlOp))
            if if_name == yamlOp['status']['name']:
                output = lif
                break

    #api.Logger.info("Returning lif output for node:%s, if_name:%s, output:%s"%(node, if_name, output))
    return retval, output

# Returns True if the given interface is a Host Lif interface
def IsHostLifIntf(yaml_output, if_name):
    lif_entries = yaml_output.split("---")

    for lif in lif_entries:
        yamlOp = yaml.load(lif, Loader=yaml.FullLoader)
        if not yamlOp:
            continue
        if if_name == yamlOp['status']['name'] and \
           yamlOp['spec']['type'] == types_pb2.LIF_TYPE_HOST: 
            return True
    return False

# Returns all host lif interfaces for all Naples
def GetHostLifInterface(tc):
    tc.host_lifs = {}
    for node in tc.nodes:
        tc.host_lifs[node] = []
        ret, lif_info_yaml = GetLifInfo(node)
        node_workloads = api.GetWorkloads(node)
        intfs = api.GetNaplesHostInterfaces(node)
        for intf in intfs:
            if IsHostLifIntf(lif_info_yaml, intf):
                wl_for_intf = ""
                for wl in node_workloads:
                    if wl.interface == intf:
                        wl_for_intf = wl.workload_name
                        break
                intf_wl = {intf:wl_for_intf}
                api.Logger.info("Retrieving Host Intfs for Node: %s, intf/wl: %s"%(node, intf_wl))
                tc.host_lifs[node].append(intf_wl)

    #api.Logger.info("Retrieved Host Lifs: %s"%(tc.host_lifs))
    #for key, items in tc.host_lifs.items():
    #    api.Logger.info("Node: %s, HostLifs(intf:wl) --> %s"%(key, items))

    return api.types.status.SUCCESS

# Checks lif status for a given host interface
def CheckLifStatus(yaml_output, if_name, status):
    result = api.types.status.SUCCESS
    lif_entries = yaml_output.split("---")
    lif_uuid = None
    for lif in lif_entries:
        yamlOp = yaml.load(lif, Loader=yaml.FullLoader)
        if not yamlOp:
            continue
        if yamlOp['spec']['type'] != types_pb2.LIF_TYPE_HOST: 
            continue
        if if_name == yamlOp['status']['name'] and \
           status != yamlOp['status']['status']:
            result = api.types.status.FAILURE
            lif_uuid = yaml_utils.List2UuidStr(yaml_utils.GetYamlSpecAttr(yamlOp['spec'], 'id'))
            break
    return result, lif_uuid

# Validates the lif status for all host interfaces
def ValidateLifStatus(tc, status):
    result = api.types.status.SUCCESS

    for node in tc.nodes:
        ret, lif_info_yaml = GetLifInfo(node)
        for intf_wl in tc.host_lifs[node]:
            intf, wl = next(iter(intf_wl.items()))
            #api.Logger.info("Checking interface %s, status %s"%(intf, status))
            ret, lif_uuid = CheckLifStatus(lif_info_yaml, intf, status)
            if ret != api.types.status.SUCCESS:
                api.Logger.info("Failed Lif status check for %s/%s, "
                     "uuid: %s,  status is not: %s"%(node, intf, lif_uuid, status))
                result = api.types.status.FAILURE
    return result

# Change the admin status of all host interfaces
def ChangeHostLifsAdminStatus(tc, shutdown=False):
    result = api.types.status.SUCCESS
    req = api.Trigger_CreateAllParallelCommandsRequest()

    tc.cmd_cookies = []
    for node in tc.nodes:
        for intf_wl in tc.host_lifs[node]:
            intf, wl = next(iter(intf_wl.items()))
            # Change admin status
            cmd = "ifconfig %s %s" % (intf, ("down" if shutdown else "up"))
            if wl:
                cmd_cookie = "Node: %s, WL: %s, intf: %s, shutdown: %s" % (node, wl, intf, shutdown)
                tc.cmd_cookies.append(cmd_cookie)
                #api.Logger.info("%s"%(cmd_cookie))
                api.Trigger_AddCommand(req, node, wl, cmd)
            else:
                cmd_cookie = "Node: %s, intf: %s, shutdown: %s" % (node, intf, shutdown)
                tc.cmd_cookies.append(cmd_cookie)
                #api.Logger.info("%s"%(cmd_cookie))
                api.Trigger_AddHostCommand(req, node, cmd)

    tc.resp = api.Trigger(req)

    cookie_idx = 0
    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
            api.Logger.info("Failed to change Admin for %s" % (tc.cmd_cookies[cookie_idx]))
            api.PrintCommandResults(cmd)
        cookie_idx += 1
    return result


def Setup(tc):
    tc.skip = False
    tc.nodes = api.GetNaplesHostnames()
    GetHostLifInterface(tc)
    api.Logger.info("Validating LIF status before trigger")
    status = types_pb2.ADMIN_STATE_ENABLE
    if ValidateLifStatus(tc, status) != api.types.status.SUCCESS:
        tc.skip = True
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):

    if tc.skip == True:
        return api.types.status.SUCCESS 

    result = api.types.status.SUCCESS 
    api.Logger.info("Shutdown all Host LIF's")
    if ChangeHostLifsAdminStatus(tc, True) != api.types.status.SUCCESS:
        api.Logger.error("Failed to shutdown one or more Host Lifs")

    status = types_pb2.ADMIN_STATE_DISABLE

    api.Logger.info("Validating LIF status after admin down")
    if ValidateLifStatus(tc, status) != api.types.status.SUCCESS:
        api.Logger.error("Failed in validating LIF status after admin down")
        result = api.types.status.FAILURE

    api.Logger.info("Admin UP all Host LIF's")
    if ChangeHostLifsAdminStatus(tc, False) != api.types.status.SUCCESS:
        api.Logger.error("Failed to bring UP one or more Host Lifs")

    if result != api.types.status.FAILURE:
        api.Logger.info("Validating LIF status after admin up")
        status = types_pb2.ADMIN_STATE_ENABLE
        if ValidateLifStatus(tc, status) != api.types.status.SUCCESS:
            api.Logger.error("Failed in validating LIF status after admin up")
            result = api.types.status.FAILURE

    return result


def Verify(tc):
    return api.types.status.SUCCESS


def Teardown(tc):
    return api.types.status.SUCCESS
