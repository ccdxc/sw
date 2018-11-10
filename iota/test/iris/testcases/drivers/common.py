#! /usr/bin/python3
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder

feature_cmd_map = {
    "tx_ring_size"       : cmd_builder.ethtool_tx_ring_size,
    "rx_ring_size"       : cmd_builder.ethtool_rx_ring_size,
    "tx_queue_size"      : cmd_builder.ethtool_tx_queue_size,
    "rx_queue_size"      : cmd_builder.ethtool_rx_queue_size,
    "tx_check_sum"       : cmd_builder.ethtool_tx_checksum,
    "rx_check_sum"       : cmd_builder.ethtool_tx_checksum,
    "tso_offload"        : cmd_builder.ethtool_tso_offload,
    "sg_offload"         : cmd_builder.ethtool_sg_offload,
    "filter_mcast"       : cmd_builder.ip_link_filter_mcast,
    "filter_all_mcast"   : cmd_builder.ip_link_filter_all_mcast,
    "filter_broadcast"   : cmd_builder.ip_link_filter_broadcast,
    "filter_promiscuous" : cmd_builder.ip_link_filter_promiscuous,
    "mtu"                : cmd_builder.ip_link_mtu_cmd,
    "rx_vlan"            : cmd_builder.ethtool_rxvlan_offload,
    "tx_vlan"            : cmd_builder.ethtool_txvlan_offload,
}

def setup_features(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    nodes = api.GetWorkloadNodeHostnames()
    for n in nodes:
        host_intfs = api.GetWorkloadNodeHostInterfaces(n)
        if len(host_intfs) == 0:
            api.Logger.error("No host interfaces for node :%s" %n)
            return api.types.status.FAILURE
        for intf in host_intfs:
            for feature, cmdBuilder in feature_cmd_map.items():
                feature_value = getattr(tc.iterators, feature, None)
                if not feature_value:
                    api.Logger.error("Feature %s not provided " % feature)
                    continue
                if not feature_value:
                    api.Logger.error("Feature %s has no value " % feature)
                    continue
                cmd = cmdBuilder(intf, feature_value)
                api.Trigger_AddHostCommand(req, n, cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error("Error running cmd : %s " % cmd.command)
            api.Logger.error("Std Output : %s " % cmd.stdout)
            api.Logger.error("Std Err :  %s "% cmd.stdout)
            return api.types.status.FAILURE
        api.Logger.info("Success running cmd : %s" % cmd.command)
    return api.types.status.SUCCESS

def pcap_file_name(tc, prefix):
    return tc.GetTcID() + "_" + prefix + ".pcap"

def start_pcap_capture(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    nodes = api.GetWorkloadNodeHostnames()
    tc.pcap_cmds = []
    for n in nodes:
        host_intfs = api.GetWorkloadNodeHostInterfaces(n)
        if len(host_intfs) == 0:
            api.Logger.error("No host interfaces for node :%s" %n)
            return api.types.status.FAILURE
        for intf in host_intfs:
            cmd = cmd_builder.tcpdump_cmd(intf, pcap_file_name(tc, intf))
            api.Trigger_AddHostCommand(req, n, cmd, background = True)


    resp = api.Trigger(req)
    for cmd in resp.commands:
        if cmd.handle == None or len(cmd.handle) == 0:
            api.Logger.error("Error starting pcap : %s " % cmd.command)
            api.Logger.error("Std Output : %s "% cmd.stdout)
            api.Logger.error("Std Err :  %s "% cmd.stdout)
            return api.types.status.FAILURE
        api.Logger.info("Success running cmd : %s" % cmd.command)
    tc.pcap_trigger = resp
    return api.types.status.SUCCESS

def stop_pcap_capture(tc):
    api.Trigger_TerminateAllCommands(tc.pcap_trigger)
    nodes = api.GetWorkloadNodeHostnames()
    tc_dir = tc.GetLogsDir()
    for n in nodes:
        host_intfs = api.GetWorkloadNodeHostInterfaces(n)
        if len(host_intfs) == 0:
            api.Logger.error("No host interfaces for node :%s" %n)
            return api.types.status.FAILURE
        for intf in host_intfs:
            resp = api.CopyFromHost(n, [pcap_file_name(tc, intf)], tc_dir)
            if resp == None or resp.api_response.api_status != types_pb2.API_STATUS_OK:
                api.Logger.error("Failed to copy from  to Node: %s" % n)
                return api.types.status.FAILURE

    return api.types.status.SUCCESS
