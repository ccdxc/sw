import iota.harness.api as api
import iota.test.iris.utils.host as host_utils
import iota.test.iris.utils.naples as naples_utils
import iota.test.utils.naples_host as naples_host_utils
import iota.test.iris.utils.hal_show as hal_show_utils
import iota.test.utils.ionic_utils as ionic_utils
import yaml
import ipaddress

def Setup(tc):

    result = api.types.status.SUCCESS
    tc.skip = False
    node_names = api.GetWorkloadNodeHostnames()

    if not api.RunningOnSameSwitch():
        tc.skip = True
        api.Logger.error("Rx Mode MC : Setup -> Multi switch topology not supported yet - So skipping the TC")
        return api.types.status.IGNORED

    if  api.IsNaplesNode(node_names[0]):
        tc.naples_node = node_names[0]
        tc.peer_node = node_names[1]
    elif api.IsNaplesNode(node_names[1]):
        tc.naples_node = node_names[1]
        tc.peer_node = node_names[0]
    else:
        api.Logger.info("Skipping as there are no Naples nodes")
        tc.skip = True
        return api.types.status.IGNORED

    if tc.args.mode != "enable_allmulti" and tc.args.mode != "disable_allmulti":
        api.Logger.error("Unknown mode '%s'. Skipping testcase" %(tc.args.mode))
        tc.skip = True
        return api.types.status.IGNORED

    if api.GetNodeOs(tc.naples_node) != "linux" and tc.args.mode == "enable_allmulti":
        api.Logger.info("Skipping testcase because allmulti cannot be set")
        tc.skip = True
        return api.types.status.SUCCESS

    tc.expect_pkt = {}
    tc.on_host = {}

    tc.host_intfs = list(api.GetNaplesHostInterfaces(tc.naples_node))
    # Unknown Multicast Packets from uplink will reach host interface when allmulti is enabled
    for intf in tc.host_intfs:
        tc.expect_pkt[intf] = True
        tc.on_host[intf] = True

    # Mgmt interface on host for network connection to Naples over PCIE (Subset of tc.host_intfs)
    tc.host_int_intfs = naples_host_utils.GetHostInternalMgmtInterfaces(tc.naples_node)
    for intf in tc.host_int_intfs:
        # Host internal management should not receive unknown multicast packets from uplink regardless of its allmulti state
        tc.expect_pkt[intf] = False
        tc.on_host[intf] = True

    tc.inband_intfs = naples_host_utils.GetNaplesInbandInterfaces(tc.naples_node)
    # Unknwown multicast packets from uplink will reach inband interface when allmulti is enabled
    for intf in tc.inband_intfs:
        tc.expect_pkt[intf] = True
        tc.on_host[intf] = False

    tc.naples_int_mgmt_intfs = naples_host_utils.GetNaplesInternalMgmtInterfaces(tc.naples_node)
    # Packets from uplink should not reach naples internal managment interfaces [int_mnic0] regardless of its allmulti
    for intf in tc.naples_int_mgmt_intfs:
        tc.expect_pkt[intf] = False
        tc.on_host[intf] = False

    tc.naples_oob_mgmt_intfs = naples_host_utils.GetNaplesOobInterfaces(tc.naples_node)
    # Packets from uplink should not reach naples oob managment interfaces [oob_mnic0] regardless of its allmulti state
    for intf in tc.naples_oob_mgmt_intfs:
        tc.expect_pkt[intf] = False
        tc.on_host[intf] = False

    tc.all_intfs = tc.host_intfs + tc.host_int_intfs + tc.inband_intfs + tc.naples_int_mgmt_intfs + tc.naples_oob_mgmt_intfs

    # In non-promiscuous mode, unknown unicast traffic shouldn't reach any interface
    if tc.args.mode == "disable_allmulti":
        for intf in tc.all_intfs:
            tc.expect_pkt[intf] = False

    api.Logger.debug("Test interfaces: ", tc.all_intfs)

    workloads = api.GetWorkloads()
    tc.peer_workloads = []

    # List of 'default vlan' workloads on peer node
    for workload in workloads:
        if workload.encap_vlan == 0 and workload.node_name == tc.peer_node:
                tc.peer_workloads.append(workload)

    # Random Multicast IP address
    tc.target_multicast_IP = "226.1.2.3"
    api.Logger.debug("Random Multicast IP = %s " %(tc.target_multicast_IP))

    # Move all interfaces to allmulti mode if tc.args.mode == 'enable_allmulti'
    if tc.args.mode == "enable_allmulti":
        for intf in tc.all_intfs:
            result = __Toggle_AllMulti(tc, intf, True)
            if result != api.types.status.SUCCESS:
                api.Logger.Error("Skipping testcase")
                break

    return result

def __Toggle_AllMulti(tc, intf, enable):

    state = "enable" if enable else "disable"

    if tc.on_host[intf]:
        #TODO: This works only for Linux.
        if enable:
            result = host_utils.EnableAllmulti(tc.naples_node, intf)
        else:
            result = host_utils.DisableAllmulti(tc.naples_node, intf)
    else:
        if enable:
            result = naples_utils.EnableAllmulti(tc.naples_node,intf)
        else:
            result = naples_utils.DisableAllmulti(tc.naples_node, intf)

    if result != api.types.status.SUCCESS:
        api.Logger.error("Failed to %s allmulti on %s" %(state, intf))

    return result

# Helper function to associate command execution target (Host / Naples Card) based on interface name
def __PR_AddCommand(intf, tc, req, cmd, bg):

    if tc.on_host[intf]:
        api.Trigger_AddHostCommand(req, tc.naples_node, cmd, bg)
    else:
        api.Trigger_AddNaplesCommand(req, tc.naples_node, cmd, bg)

def Trigger(tc):

    if tc.skip: return api.types.status.SUCCESS
    result = api.types.status.SUCCESS
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    # Run tcpdump in non-promiscuous mode on all interfaces
    for intf in tc.all_intfs:
        if api.GetNodeOs(tc.naples_node) == "windows" and intf in tc.host_intfs:
            intfGuid = ionic_utils.winIntfGuid(tc.naples_node, intf)
            intfVal = str(ionic_utils.winTcpDumpIdx(tc.naples_node, intfGuid))
        else:
            intfVal = intf

        cmd = "tcpdump -l -i " + intfVal  + " -ptne  host " + tc.target_multicast_IP
        if api.GetNodeOs(tc.naples_node) == "windows" and intf in tc.host_intfs:
            cmd = "/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe  \" " + cmd + ";sleep 10 \""
        __PR_AddCommand(intf, tc, req, cmd, True)


    if api.GetNodeOs(tc.naples_node) == "windows":
        cmd = "/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe  \" sleep 10; ping -n 5 -S " + tc.peer_workloads[0].ip_address + " " + tc.target_multicast_IP + ";sleep 10 \" "
    else:
        cmd = "sleep 1; ping -c 5 -I " + tc.peer_workloads[0].ip_address + " " + tc.target_multicast_IP + ";sleep 1"

    api.Trigger_AddHostCommand(req, tc.peer_node, cmd)
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    # Verify packet filter flags of each interface in halctl
    show_lif_resp, ret = hal_show_utils.GetHALShowOutput(tc.naples_node, "lif")
    if not ret:
        api.Logger.error("Something went wrong with GetHALShowOutput")
        result = api.types.status.FAILURE

    lif_obj_docs = yaml.load_all(show_lif_resp.commands[0].stdout, Loader=yaml.FullLoader)

    for lif_obj in lif_obj_docs:

        if lif_obj == None:
            break

        # See if the lif belongs to any of the interfaces in tc.all_intfs (inteface lif)
        intf_lif = False
        for intf in tc.all_intfs:
            if api.GetNodeOs(tc.naples_node) == "windows" and intf in tc.host_intfs:
                halIntfName = ionic_utils.winHalIntfName(tc.naples_node, intf)
            else:
                halIntfName = intf
            if lif_obj['spec']['name'].startswith(halIntfName):
                intf_lif = True
                break
        lif_am_flag = lif_obj['spec']['packetfilter']['receiveallmulticast']

        # A lif must have its AM flag set when it is an interface lif and tc.args.mode is 'enable_allmulti'
        if intf_lif == True:
            if tc.args.mode == "enable_allmulti" and lif_am_flag != True:
                api.Logger.error("halctl AM flag not set for allmulti mode interface [%s]" %(lif_obj['spec']))
                result = api.types.status.FAILURE
            elif tc.args.mode == "disable_allmulti" and lif_am_flag == True:
                api.Logger.error("halctl AM flag set for no-allmulti mode interface [%s]" %(lif_obj['spec']))
                result = api.types.status.FAILURE

        else:
            # Ignore for SWM lifs
            if lif_am_flag == True and lif_obj['spec']['type'] != 7:
                api.Logger.error("halctl AM flag set for LIF [%s]" %(lif_obj['spec']))
                result = api.types.status.FAILURE


    # Search tcpdump stdout for packets with dst MAC matching tc.random_mac
    pattern = "> " + tc.target_multicast_IP
    cmds = resp.commands[:-1]
    for intf, cmd in zip(tc.all_intfs, cmds):
        found = cmd.stdout.find(pattern)
        if found > 0 and not tc.expect_pkt[intf]:
            api.Logger.error("Interface [%s] received Unknown multicast packet while not expecting" %(intf))
            result = api.types.status.FAILURE
        elif found == -1 and tc.expect_pkt[intf]:
            api.Logger.error("Interface [%s] did not receive expected Unknown multicast packet" %(intf))
            result = api.types.status.FAILURE

    # Incase of testcase failure, dump the entire command output for further debug
    if result == api.types.status.FAILURE:
        api.Logger.error(" ============================= COMMAND DUMP ================================================")
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
        api.Logger.error(" =============================  END  ========================================================")

    return result

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):

    if tc.skip: return api.types.status.SUCCESS
    result = api.types.status.SUCCESS

    # Move back interfaces to no-allmulti mode if tc.args.mode == 'enable_allmulti'
    if tc.args.mode == "enable_allmulti":
        for intf in tc.all_intfs:
            result = __Toggle_AllMulti(tc, intf, False)

    return result
