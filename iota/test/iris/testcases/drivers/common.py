#! /usr/bin/python3
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.config.netagent.hw_push_config as cfg_api
import iota.test.iris.utils.naples_host as utils

feature_cmd_map = {
    "tx_ring_size"       :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_tx_ring_size,     "reloadCmd" : True },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_tx_ring_size, "reloadCmd" : True },
        },
    "rx_ring_size"       :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_rx_ring_size,     "reloadCmd" : True },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_rx_ring_size, "reloadCmd" : True },
        },
    "tx_queue_size"      :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_tx_queue_size,     "reloadCmd" : True },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_tx_queue_size, "reloadCmd" : True },
        },
    "rx_queue_size"      :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_rx_queue_size,     "reloadCmd" : True },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_rx_queue_size, "reloadCmd" : True },
        },
    "tx_check_sum"       :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_tx_checksum },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_tx_checksum }
        },
    "rx_check_sum"       :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_rx_checksum },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_rx_checksum }
        },
    "tso_offload"        :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_tso_offload },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_tso_offload }
        },
    "sg_offload"         :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_sg_offload },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_sg_offload }
        },
    "filter_mcast"       :
        {
            "linux"    : { "cmd" : cmd_builder.ip_link_filter_mcast },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ip_link_filter_mcast }
        },
    "filter_all_mcast"   :
        {
            "linux"    : { "cmd" : cmd_builder.ip_link_filter_all_mcast },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ip_link_filter_all_mcast }
        },
    "filter_broadcast"   :
        {
            "linux"    : { "cmd" : cmd_builder.ip_link_filter_broadcast },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ip_link_filter_broadcast }
        },
    "filter_promiscuous" :
        {
            "linux"    : { "cmd" : cmd_builder.ip_link_filter_promiscuous },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ip_link_filter_promiscuous }
        },
    "mtu"                :
        {
            "linux"    : { "cmd" : cmd_builder.ip_link_mtu_cmd },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ip_link_mtu_cmd }
        },
    "rx_vlan"            :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_rxvlan_offload },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_rxvlan_offload }
        },
    "tx_vlan"            :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_txvlan_offload },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_txvlan_offload }
        },
    "hkey"               :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_hkey_offload },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_hkey_offload }
        },
    "indirection_table"  :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_indirection_table },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_indirection_table }
        },
    "rx-flow-hash"     :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_rssflow },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_rssflow }
        },
    "intr-coal"     :
        {
            "linux"    : { "cmd" : cmd_builder.ethtool_intr_coal },
            "freebsd"  : { "cmd" : cmd_builder.bsd_ethtool_intr_coal }
        },
}

def setup_features(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    driverReloaded = False
    for intfObj in tc.test_intfs:
        intf = intfObj.Name()
        n = intfObj.Node()
        reloadDone = False
        for feature, cmdBuilderDict in feature_cmd_map.items():
            feature_value = getattr(tc.iterators, feature, None)
            if not feature_value:
                api.Logger.error("Feature %s not provided " % feature)
                continue
            os_type = intfObj.OsType()
            callback = cmdBuilderDict[os_type]["cmd"]
            cmds = callback(n, intf, feature_value)
            if not isinstance(cmds, list):
                cmds = [cmds]
            for cmd in cmds:
                intfObj.AddCommand(req, cmd)
            if cmdBuilderDict[os_type].get("reloadCmd"):
                #Driver reload, just break as no need to setup for each interface.
                if reloadDone:
                    api.Logger.error("Driver reload already added for feature, can't do it again")
                    assert(0)
                reloadDone = True
                driverReloaded = True
                break

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Error running cmd : %s " % cmd.command)
            api.Logger.error("Std Output : %s " % cmd.stdout)
            api.Logger.error("Std Err :  %s "% cmd.stdout)
            if api.IsNaplesNode(cmd.node_name):
                return api.types.status.FAILURE
            else:
                api.Logger.info("Ignoring cmd error its non-naples node : %s" % cmd.command)

        api.Logger.info("Success running cmd : %s" % cmd.command)

    if driverReloaded:
        for intfObj in tc.test_intfs:
            ipproto = getattr(tc.iterators, "ipproto", 'v4')
            intfObj.ReconfigureInterface(ipproto)

    return api.types.status.SUCCESS

def pcap_file_name(prefix):
    return  prefix + ".pcap"

def start_pcap_capture(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    nodes = api.GetWorkloadNodeHostnames()
    tc.pcap_cmds = []
    for n, host_intfs in tc.host_intfs.items():
        if len(host_intfs) == 0:
            api.Logger.error("No host interfaces for node :%s" %n)
            return api.types.status.FAILURE
        for intfObj in host_intfs:
            intf = intfObj.Name()
            cmd = cmd_builder.tcpdump_cmd(intf, pcap_file_name(intf))
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
    for n, host_intfs in tc.host_intfs.items():
        if len(host_intfs) == 0:
            api.Logger.error("No host interfaces for node :%s" %n)
            return api.types.status.FAILURE
        for intfObj in host_intfs:
            intf = intfObj.Name()
            resp = api.CopyFromHost(n, [pcap_file_name(intf)], tc_dir)
            if resp == None or resp.api_response.api_status != types_pb2.API_STATUS_OK:
                api.Logger.error("Failed to copy from  to Node: %s" % n)
                return api.types.status.FAILURE

    return api.types.status.SUCCESS
