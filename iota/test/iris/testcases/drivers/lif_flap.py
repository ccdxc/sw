#!/usr/bin/python3
import yaml

import iota.harness.api as api
import iota.test.iris.utils.hal_show as halctl

from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

# Returns output of halctl show lif
def GetLifInfo(node, if_name=None):
    if GlobalOptions.dryrun:
        return True, ''
    output, retval = halctl.GetHALShowOutput(node, 'lif', yaml=True)

    if not retval:
        api.Logger.error("Failed to get lif info from halctl")
        output = ''
        return retval, output

    output = output.commands[0].stdout

    if if_name:
        # split output and fetch the info for given if_name
        lif_entries = output.split("---")
        for lif in lif_entries:
            yamlOp = yaml.load(lif, Loader=yaml.FullLoader)
            if not yamlOp:
                continue
            #api.Logger.info("LIF Yaml Output for ifname:%s, output:%s"%(if_name, yamlOp))
            if if_name == yamlOp['spec']['name']:
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
        if if_name == yamlOp['spec']['name'] and \
           yamlOp['spec']['type'] == 1: #types_pb2.LIF_TYPE_HOST(1) or LIF_TYPE_MNIC_INBAND_MANAGEMENT(5)
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
        #api.Logger.info("Host Intfs for Node: %s, intf: %s"%(node, intfs))
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
    lif_id = None
    for lif in lif_entries:
        yamlOp = yaml.load(lif, Loader=yaml.FullLoader)
        if not yamlOp:
            continue
        if yamlOp['spec']['type'] != 1: #types_pb2.LIF_TYPE_HOST(1) or LIF_TYPE_MNIC_INBAND_MANAGEMENT(5) 
            continue
        if if_name == yamlOp['spec']['name'] and \
           status != yamlOp['spec']['adminstatus']:
            result = api.types.status.FAILURE
            lif_d = yamlOp['spec']['keyorhandle']['keyorhandle']['lifid']
            break
    return result, lif_id

# Validates the lif status for all host interfaces
def ValidateLifStatus(tc, status):
    result = api.types.status.SUCCESS

    for node in tc.nodes:
        ret, lif_info_yaml = GetLifInfo(node)
        for intf_wl in tc.host_lifs[node]:
            intf, wl = next(iter(intf_wl.items()))
            #api.Logger.info("Checking interface %s, status %s"%(intf, status))
            ret, lif_id = CheckLifStatus(lif_info_yaml, intf, status)
            if ret != api.types.status.SUCCESS:
                api.Logger.info("Failed Lif status check for %s/%s, "
                     "Lif Id: %s,  status is not: %s"%(node, intf, lif_id, status))
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
    status = 1 #interface_pb2.IF_STATUS_UP
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

    status = 2 #interface_pb2.IF_STATUS_DOWN

    api.Logger.info("Validating LIF admin status after admin down")
    if ValidateLifStatus(tc, status) != api.types.status.SUCCESS:
        api.Logger.error("Failed in validating LIF admin status after admin down")
        result = api.types.status.FAILURE

    api.Logger.info("Admin UP all Host LIF's")
    if ChangeHostLifsAdminStatus(tc, False) != api.types.status.SUCCESS:
        api.Logger.error("Failed to bring UP one or more Host Lifs")

    if result != api.types.status.FAILURE:
        api.Logger.info("Validating LIF admin status after admin up")
        status = 1 #interface_pb2.IF_STATUS_UP
        if ValidateLifStatus(tc, status) != api.types.status.SUCCESS:
            api.Logger.error("Failed in validating LIF admin status after admin up")
            result = api.types.status.FAILURE

    return result


def Verify(tc):
    return api.types.status.SUCCESS


def Teardown(tc):
    return api.types.status.SUCCESS
