import iota.harness.api as api
import iota.test.iris.utils.host as host_utils
import iota.test.iris.utils.naples as naples_utils
import iota.test.utils.naples_host as naples_host_utils
import iota.test.iris.utils.hal_show as hal_show_utils
import iota.test.utils.ionic_utils as ionic_utils
import yaml
import ipaddress

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

    tc.expect_pkt = {}
    tc.on_host = {}

    tc.host_intfs = list(api.GetNaplesHostInterfaces(tc.naples_node))
    # UUC Packets from uplink will reach host interface while promiscuous
    for intf in tc.host_intfs:
        tc.expect_pkt[intf] = True
        tc.on_host[intf] = True

    # Mgmt interface on host for network connection to Naples over PCIE
    tc.host_int_intfs = naples_host_utils.GetHostInternalMgmtInterfaces(tc.naples_node)
    for intf in tc.host_int_intfs:
        # Host internal management should not receive packets from uplink regardless of its promiscuity state
        tc.expect_pkt[intf] = False
        tc.on_host[intf] = True

    tc.inband_intfs = naples_host_utils.GetNaplesInbandInterfaces(tc.naples_node)
    # UUC Packets from uplink will reach inabnd interface while promiscuous
    for intf in tc.inband_intfs:
        tc.expect_pkt[intf] = True
        tc.on_host[intf] = False

    tc.naples_int_mgmt_intfs = naples_host_utils.GetNaplesInternalMgmtInterfaces(tc.naples_node)
    # Packets from uplink should not reach naples internal managment interfaces [int_mnic0] regardless of its promiscuity state
    for intf in tc.naples_int_mgmt_intfs:
        tc.expect_pkt[intf] = False
        tc.on_host[intf] = False

    tc.naples_oob_mgmt_intfs = naples_host_utils.GetNaplesOobInterfaces(tc.naples_node)
    # Packets from uplink should not reach naples oob managment interfaces [oob_mnic0] regardless of its promiscuity state
    for intf in tc.naples_oob_mgmt_intfs:
        tc.expect_pkt[intf] = False
        tc.on_host[intf] = False

    tc.all_intfs = tc.host_intfs + tc.host_int_intfs + tc.inband_intfs + tc.naples_int_mgmt_intfs + tc.naples_oob_mgmt_intfs
    api.Logger.info("Promiscuous test interfaces: ", tc.all_intfs)

    workloads = api.GetWorkloads()
    tc.peer_workloads = []

    # List of 'default vlan' workloads on peer node
    for workload in workloads:
        if workload.encap_vlan == 0 and workload.node_name == tc.peer_node:
                tc.peer_workloads.append(workload)


    # Random IP address within workload[0] IP address domain
    tc.target_IP = (ipaddress.ip_address(tc.peer_workloads[0].ip_address) + 2).__str__()

    #TODO: Generate a random MAC instead
    tc.random_mac = "00:0f:b7:aa:bb:cc"

    api.Logger.info("Random_Ip = %s Random_MAC = %s " %(tc.target_IP, tc.random_mac))

    host_utils.DeleteARP(tc.peer_node, tc.peer_workloads[0].interface, tc.target_IP)
    if  host_utils.AddStaticARP(tc.peer_node, tc.peer_workloads[0].interface, tc.target_IP, tc.random_mac) != api.types.status.SUCCESS:
        api.Logger.error("Failed to add Static ARP entry on %s" %(tc.peer_node))
        return api.types.status.FAILURE
    else:
        api.Logger.info("Added Random_Ip = %s Random_MAC = %s on %s" %(tc.target_IP, tc.random_mac, tc.peer_workloads[0].interface))
    return api.types.status.SUCCESS


# Helper function to associate command execution target (Host / Naples Card) based on interface name
def __PR_AddCommand(intf, tc, req, cmd, bg):

    if tc.on_host[intf]:
        api.Trigger_AddHostCommand(req, tc.naples_node, cmd, bg)
    else:
        api.Trigger_AddNaplesCommand(req, tc.naples_node, cmd, bg)

def Trigger(tc):

    if tc.skip: return api.types.status.SUCCESS
    result = api.types.status.SUCCESS

    for intf1 in tc.all_intfs:

        if result == api.types.status.FAILURE:
            break
        #  Run tcpdump on intf1 in promiscuous mode
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        if api.GetNodeOs(tc.naples_node) == "windows" and intf1 in tc.host_intfs:
            cmd = "/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe tcpdump.exe "
            intfGuid = ionic_utils.winIntfGuid(tc.naples_node, intf1)
            intfVal = str(ionic_utils.winTcpDumpIdx(tc.naples_node, intfGuid))
        else:
            intfVal = intf1
            
        cmd = "tcpdump -l -i " + intfVal + " -tne ether host " + tc.random_mac
        if api.GetNodeOs(tc.naples_node) == "windows":
            cmd = "/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe  \" " + cmd + " \""
        __PR_AddCommand(intf1, tc, req, cmd, True)

        # Run tcpdump in non-promiscuous mode on all other interfaces
        for intf2 in tc.all_intfs:
            if intf2 != intf1:
                if api.GetNodeOs(tc.naples_node) == "windows" and intf2 in tc.host_intfs:
                    intfGuid = ionic_utils.winIntfGuid(tc.naples_node, intf2)
                    intfVal = str(ionic_utils.winTcpDumpIdx(tc.naples_node, intfGuid))
                else:
                    intfVal = intf2
                
                cmd = "tcpdump -l -i " + intfVal + " -ptne ether host " + tc.random_mac
                if api.GetNodeOs(tc.naples_node) == "windows" and intf2 in tc.host_intfs:
                    cmd = "/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe  \" " + cmd + " \""
                __PR_AddCommand(intf2, tc, req, cmd, True)


        if api.GetNodeOs(tc.naples_node) == "windows":
            cmd = "/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe  \" sleep 10; ping -n 5 " + tc.target_IP + ";sleep 10 \" "
        else:
            cmd = "sleep 1; ping -c 5 " + tc.target_IP + ";sleep 1"

        api.Trigger_AddHostCommand(req, tc.peer_node, cmd)
        cmds_terminated = False
        trig_resp = api.Trigger(req)

        # Verify packet filter flags of each interface in halctl
        show_lif_resp, ret = hal_show_utils.GetHALShowOutput(tc.naples_node, "lif")
        if not ret:
            api.Logger.error("Something went wrong with GetHALShowOutput")
            result = api.types.status.FAILURE
            break
        lif_obj_docs = yaml.load_all(show_lif_resp.commands[0].stdout, Loader=yaml.Loader)

        for lif_obj in lif_obj_docs:
            if lif_obj == None:
                break
            if api.GetNodeOs(tc.naples_node) == "windows":
                halIntfName = ionic_utils.winHalIntfName(tc.naples_node, intf1)
            else:
                halIntfName = intf1
                
            if lif_obj['spec']['name'].startswith(halIntfName):
                if lif_obj['spec']['packetfilter']['receivepromiscuous'] != True:
                    api.Logger.error("halctl PR flag not set for promiscuous mode interface [%s]" %(intf1))
                    result = api.types.status.FAILURE
            else:
                if lif_obj['spec']['packetfilter']['receivepromiscuous'] == True:
                    api.Logger.error("halctl PR flag set for non-promiscuous mode interface [%s]" %(lif_obj['spec']))
                    result = api.types.status.FAILURE

        term_resp = api.Trigger_TerminateAllCommands(trig_resp)
        resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
        cmds_terminated = True

        # Search tcpdump stdout for packets with dst MAC matching tc.random_mac
        pattern = "> " + tc.random_mac

        # Parse tcpdump stdout of promiscuous mode interface
        found = resp.commands[0].stdout.find(pattern)
        if tc.expect_pkt[intf1] and found == -1:
            api.Logger.error("Interface [%s] did not receive Unknown Unicast packet in promiscuous mode IP(%s) MAC(%s)" 
                             %(intf1, tc.target_IP, tc.random_mac))
            api.PrintCommandResults(resp.commands[0])
            result = api.types.status.FAILURE
        elif not tc.expect_pkt[intf1] and found > 0:
            api.Logger.error("Interface [%s] received packet in promiscuous mode while not expecting" %(intf1))
            api.PrintCommandResults(resp.commands[0])
            result = api.types.status.FAILURE

        # Parse tcpdump stdout of rest of the interfaces
        cmds = resp.commands[1:]
        for cmd in cmds:
            found = cmd.stdout.find(pattern)
            if found > 0:
                api.Logger.error("Interface in non-promiscuous mode received unknown unicast packet")
                api.PrintCommandResults(cmd)
                result = api.types.status.FAILURE
                break

    #    api.Logger.info(" ==================  DUMP FOR %s   ================================================================" %(intf1))
    #    for cmd in resp.commands:
    #        api.PrintCommandResults(cmd)
    #    api.Logger.info(" ==================  COMPLETED ITERATION FOR %s ====================================================" %(intf1))

    if not cmds_terminated:
        api.Trigger_TerminateAllCommands(trig_resp)
    return result


def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    if tc.skip: return api.types.status.SUCCESS
    # Remove Static ARP entry added in Setup
    return host_utils.DeleteARP(tc.peer_node, tc.peer_workloads[0].interface, tc.target_IP)
