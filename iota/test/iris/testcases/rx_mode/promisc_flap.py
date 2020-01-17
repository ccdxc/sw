import iota.harness.api as api
import iota.test.iris.utils.host as host_utils
import iota.test.iris.utils.naples as naples_utils
import iota.test.utils.naples_host as naples_host_utils
import iota.test.iris.utils.traffic as traffic
import iota.test.iris.utils.hal_show as hal_show_utils
import yaml
import time

def Setup(tc):

    tc.skip = False
    node_names = api.GetWorkloadNodeHostnames()

    if  api.IsNaplesNode(node_names[0]):
        tc.naples_node = node_names[0]
        tc.peer_node = node_names[1]
    elif api.IsNaplesNode(node_names[1]):
        tc.naples_node = node_names[1]
        tc.peer_node = node_names[0]
    else:
        api.Logger.verbose("Skipping as there are no Naples nodes")
        tc.skip = True
        return api.types.status.IGNORED

    tc.on_host = {}

    tc.host_intfs = list(api.GetNaplesHostInterfaces(tc.naples_node))
    for intf in tc.host_intfs:
        tc.on_host[intf] = True

    # Mgmt interface on host for network connection to Naples over PCIE
    tc.host_int_intfs = naples_host_utils.GetHostInternalMgmtInterfaces(tc.naples_node)
    for intf in tc.host_int_intfs:
        tc.on_host[intf] = True

    tc.inband_intfs = naples_host_utils.GetNaplesInbandInterfaces(tc.naples_node)
    for intf in tc.inband_intfs:
        tc.on_host[intf] = False

    tc.naples_int_mgmt_intfs = naples_host_utils.GetNaplesInternalMgmtInterfaces(tc.naples_node)
    for intf in tc.naples_int_mgmt_intfs:
        tc.on_host[intf] = False

    tc.naples_oob_mgmt_intfs = naples_host_utils.GetNaplesOobInterfaces(tc.naples_node)
    for intf in tc.naples_oob_mgmt_intfs:
        tc.on_host[intf] = False

    tc.all_intfs = tc.host_intfs + tc.host_int_intfs + tc.inband_intfs + tc.naples_int_mgmt_intfs + tc.naples_oob_mgmt_intfs
    api.Logger.debug("Promiscuous test interfaces: ", tc.all_intfs)

    return api.types.status.SUCCESS

def verify_hal_output(tc, is_promisc):

    result = api.types.status.SUCCESS
    show_lif_resp, ret = hal_show_utils.GetHALShowOutput(tc.naples_node, "lif")
    if not ret:
        api.Logger.error("Something went wrong with GetHALShowOutput")
        result = api.types.status.FAILURE

    lif_obj_docs = yaml.load_all(show_lif_resp.commands[0].stdout)

    for lif_obj in lif_obj_docs:

        if lif_obj == None:
            break
        lif_pr_flag = lif_obj['spec']['packetfilter']['receivepromiscuous']

        if is_promisc == False:
            if lif_pr_flag == False:
                continue
            else:
                api.Logger.error("halctl PR flag set for non-promiscuous mode LIF [%s]" %(lif_obj['spec']['name']))
                result = api.types.status.FAILURE
        else:
            intf_lif = False
            for intf in tc.all_intfs:
                if lif_obj['spec']['name'].startswith(intf):
                    intf_lif = True
                    break
            # If LIF does not belong to any of the software interfaces, it's PR flag must be False
            if intf_lif == False and lif_pr_flag == True:
                api.Logger.error("Halctl PR flag set for non-promiscuous mode LIF [%s]" %(lif_obj['spec']['name']))
                result = api.types.status.FAILURE
            elif intf_lif == True and lif_pr_flag != True:
                api.Logger.error("halctl PR flag not set for promiscuous mode interface [%s]" %(lif_obj['spec']['name']))
                result = api.types.status.FAILURE
    return result

def Trigger(tc):

    if tc.skip: return api.types.status.SUCCESS

    # Initially, verify the state of the testbed. None of the LIFs should be in PR mode
    if verify_hal_output(tc, False) != api.types.status.SUCCESS:
        api.Logger.info("Testbed does not seem to be in sane state")
        tc.skip = True
        return api.types.status.FAILURE

    # Now toggle the PR flag of all interfaces
    api.Logger.info("Moving all interfaces to promiscuous mode")
    for intf in tc.all_intfs:
        if tc.on_host[intf]:
            result = host_utils.EnablePromiscuous(tc.naples_node, intf)
        else:
           result = naples_utils.EnablePromiscuous(tc.naples_node, intf)

        if result != api.types.status.SUCCESS:
            api.Logger.info("Failed to enable promiscuous mode on %s" %(intf))
            break
    time.sleep(30)
    # Verify that the LIF flags have changed appropriately
    if verify_hal_output(tc, True) != api.types.status.SUCCESS:
        api.Logger.info("Looks like PR flag is not set for some interfaces")
        result = api.types.status.FAILURE
        tc.skip = True

    # Now toggle back PR flag
    api.Logger.info("Moving all interfaces back to non-promiscuous mode")
    for intf in tc.all_intfs:
        if tc.on_host[intf]:
            result_1 = host_utils.DisablePromiscuous(tc.naples_node, intf)
        else:
           result_1 = naples_utils.DisablePromiscuous(tc.naples_node, intf)

        if result_1 != api.types.status.SUCCESS:
            api.Logger.info("Failed to disable promiscuous mode on %s" %(intf))
            result = api.types.status.FAILURE
            tc.skip = True

    time.sleep(30)
    # Ensure that PR flags are gone now
    if verify_hal_output(tc, False) != api.types.status.SUCCESS:
        api.Logger.info("Looks like PR flag is not gone after the flap")
        tc.skip = True
        result = api.types.status.FAILURE

    return result

def Verify(tc):
    if tc.skip: return api.types.status.SUCCESS

    api.Logger.info("Ping remote workload pairs")
    ping_cookies, ping_resp = traffic.pingAllRemoteWloadPairs()
    return traffic.verifyPing(ping_cookies, ping_resp)

def Teardown(tc):
    return api.types.status.SUCCESS
