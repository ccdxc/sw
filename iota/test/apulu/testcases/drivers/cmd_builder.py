#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.naples as naples
import iota.test.utils.naples_host as host

symmetric_key = "6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A:6D:5A"

msft_key = "6d:5a:56:da:25:5b:0e:c2:41:67:25:3d:43:a3:8f:b0:d0:ca:2b:cb:ae:7b:30:b4:77:cb:2d:a3:80:30:f2:0c:6a:42:b7:3b:be:ac:01:fa"

def ethtool_feature_cmd(node, intf,feature, on_off):
    return " ".join(["ethtool", "-K", intf, feature,  on_off])

def ethtool_ring_size_cmd(node, intf,ring_type, size):
    return " ".join(["ethtool", "-G", intf, ring_type,  str(size)])

def ip_link_pkt_filter_cmd(node, intf,pkt_filter, on_off):
    return " ".join(["ip", "link", "set", "dev", intf,pkt_filter, on_off])

def ip_link_ucast_mac_filter_cmd(node, intf,mac):
    return " ".join(["ip", "link", "set", "dev", intf,"address", mac])

def ip_link_mtu_cmd(node, intf,mtu):
    return " ".join(["ip", "link", "set", "dev", intf,"mtu", mtu])

def ethtool_checksum_cmd(node, intf,feature, on_off):
    return " ".join(["ethtool", "--offload",  intf, feature,  on_off])

def ethtool_tx_ring_size(node, intf,size):
    return ethtool_ring_size_cmd(node, intf,"tx", size)

def ethtool_rx_ring_size(node, intf,size):
    return ethtool_ring_size_cmd(node, intf,"rx", size)

def ethtool_rx_sg_size(node, intf,size):
    return "echo rx SGL not suported in Linux"

def ethtool_legacy_intr_mode(node, intf, op):
    return "echo Legacy interrupt mode not supported in Linux"

def ethtool_queue_size(node, intf,size):
    return " ".join(["ethtool", "-L",  intf, "combined", str(size)])

def ethtool_tx_checksum(node, intf,op):
    return ethtool_checksum_cmd(node, intf,"tx", op)

def ethtool_rx_checksum(node, intf,op):
    return ethtool_checksum_cmd(node, intf,"rx", op)

def ethtool_tso_offload(node, intf,op):
    return ethtool_feature_cmd(node, intf,"tso", op)

def ethtool_gro_offload(node, intf,op):
    return ethtool_feature_cmd(node, intf,"gro", op)

def ethtool_sg_offload(node, intf,op):
    return ethtool_feature_cmd(node, intf,"sg", op)

def ethtool_rxvlan_offload(node, intf,op):
    return ethtool_feature_cmd(node, intf,"txvlan", op)

def ethtool_txvlan_offload(node, intf,op):
    return ethtool_feature_cmd(node, intf,"rxvlan", op)

def ethtool_hkey_offload(node, intf,op):
    if op == "symmetric":
        return " ".join(["ethtool", "-X", intf, "hkey",  symmetric_key])
    else:
        return " ".join(["ethtool", "-X", intf, "hkey",  msft_key])

def ethtool_indirection_table(node, intf,op):
    if op == "w1":
        return " ".join(["ethtool", "-X", intf, "weight 2 2 2 2 2 2"])
    elif op == "w2":
        return " ".join(["ethtool", "-X", intf, "weight 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2"])
    elif op == "equal":
        return " ".join(["ethtool", "-X", intf, "equal 4"])
    else :
        return " ".join(["ethtool", "-X",  intf, op])

def ethtool_intr_coal(node, intf, size):
    return "echo Not implemented"

def ethtool_rssflow(node, intf,op):
    return " ".join(["ethtool", "-N", intf, "rx-flow-hash",  op,  "sdfn"])

def ip_link_filter_mcast(node, intf,op):
    return ip_link_pkt_filter_cmd(node, intf,"multicast", op)

def ip_link_filter_all_mcast(node, intf,op):
    return ip_link_pkt_filter_cmd(node, intf,"allmulticast", op)

def ip_link_filter_broadcast(node, intf,op):
    return ip_link_pkt_filter_cmd(node, intf,"broadcast", op)

def ip_link_filter_promiscuous(node, intf,op):
    return ip_link_pkt_filter_cmd(node, intf,"promisc", op)

def tcpdump_cmd(node, intf,filename):
    return " ".join(["tcpdump", "-xxi", intf,"-w",  filename])

def iperf_server_cmd(port = None, time=300, run_core=None):
    assert(port)
    cmd = ["iperf3", "-s","-p", str(port)]
    if run_core:
        cmd.extend(["-A", str(run_core)])

    return " ".join(cmd)

def iperf_client_cmd(server_ip, port = None, time=10, pktsize=None, proto='tcp', run_core=None, ipproto='v4'):
    assert(port)
    cmd = ["iperf3", "-c", str(server_ip), "-p", str(port)]

    if time:
        cmd.extend(["-t", str(time)])

    if run_core:
        cmd.extend(["-A", str(run_core)])

    if proto == 'udp':
        cmd.append('-u -b 100G')

    if pktsize:
            cmd.extend(["-l", str(pktsize)])

    if ipproto == 'v6':
        cmd.append("-6")

    return " ".join(cmd)



#All Free BSD commands are built here

def bsd_ethtool_feature_cmd(node, intf,feature, on_off):
    return " ".join(["ethtool", "-K",  intf, feature,  on_off])

def bsd_ethtool_ring_size_cmd(node, intf,ring_type, size):
    args = { }
    if ring_type == "tx":
        args['hw.ionic.tx_descs'] = size
    else:
        args['hw.ionic.rx_descs'] = size
    if api.IsNaplesNode(node):
        host.UnloadDriver(host.OS_TYPE_BSD, node)
        cmds = naples.InsertIonicDriverCommands(os_type = host.OS_TYPE_BSD, **args)
        return cmds
    return " " #.join(["ethtool", "-G", ring_type,  str(size)])

def bsd_ethtool_queue_size_cmd(node, intf,queue_type, size):
    args = { }
    if queue_type == "tx":
        args['hw.ionic.max_queues'] = size
    else:
        args['hw.ionic.max_queues'] = size
    if api.IsNaplesNode(node):
        host.UnloadDriver(host.OS_TYPE_BSD, node)
        cmds = naples.InsertIonicDriverCommands(os_type = host.OS_TYPE_BSD, **args)
        return cmds
    return " " #.join(["ethtool", "-L", intf, queue_type,  str(size)])

def bsd_ethtool_rx_sg_size_cmd(node, intf, size):
    args = { }
    args['hw.ionic.rx_sg_size'] = size
    if api.IsNaplesNode(node):
        host.UnloadDriver(host.OS_TYPE_BSD, node)
        cmds = naples.InsertIonicDriverCommands(os_type = host.OS_TYPE_BSD, **args)
        return cmds
    return " "

def bsd_legacy_intr_mode_cmd(node, intf, op):
    args = { }
    if op == "on":
        args['hw.ionic.enable_msix'] = 0
    else:
        args['hw.ionic.enable_msix'] = 1
    if api.IsNaplesNode(node):
        host.UnloadDriver(host.OS_TYPE_BSD, node)
        cmds = naples.InsertIonicDriverCommands(os_type = host.OS_TYPE_BSD, **args)
        return cmds
    return " "

def bsd_ip_link_pkt_filter_cmd(node, intf,pkt_filter, on_off):
    return " ".join(["ip", "link", "set", "dev", intf,pkt_filter, on_off])

def bsd_ip_link_ucast_mac_filter_cmd(node, intf,mac):
    return " ".join(["ip", "link", "set", "dev", intf,"address", mac])

def bsd_ip_link_mtu_cmd(node, intf,mtu):
    return " ".join(["ifconfig", intf,"mtu", mtu])

def bsd_ethtool_checksum_cmd(node, intf,feature, on_off):
    return " ".join(["ethtool", "--offload", intf, feature,  on_off])

def bsd_ethtool_tx_ring_size(node, intf,size):
    return bsd_ethtool_ring_size_cmd(node, intf,"tx", size)

def bsd_ethtool_rx_ring_size(node, intf,size):
    return bsd_ethtool_ring_size_cmd(node, intf,"rx", size)

def bsd_ethtool_tx_queue_size(node, intf,size):
    return bsd_ethtool_queue_size_cmd(node, intf,"tx", size)

def bsd_ethtool_rx_queue_size(node, intf,size):
    return bsd_ethtool_queue_size_cmd(node, intf,"rx", size)

def bsd_ethtool_rx_sg_size(node, intf,size):
    return bsd_ethtool_rx_sg_size_cmd(node, intf, size)

def bsd_legacy_intr_mode(node, intf, op):
    return bsd_legacy_intr_mode_cmd(node, intf, op)

def bsd_ethtool_tx_checksum(node, intf,op):
    return " ".join(["ifconfig", intf,"txcsum" if op == "on" else "-txcsum"])

def bsd_ethtool_rx_checksum(node, intf,op):
    return " ".join(["ifconfig", intf,"rxcsum" if op == "on" else "-rxcsum"])

def bsd_ethtool_tso_offload(node, intf,op):
    return " ".join(["ifconfig", intf,"tso" if op == "on" else "-tso"])

def bsd_ethtool_intr_coal(node, intf, size):
    return "".join(["sysctl dev.ionic.0.intr_coal=", str(size)])

def bsd_ethtool_gro_offload(node, intf,op):
    return ethtool_feature_cmd(node, intf,"gro", op)

def bsd_ethtool_sg_offload(node, intf,op):
    #SG not required in freebsd
    return "echo > /dev/null"
    #return ethtool_feature_cmd(node, intf,"sg", op)

def bsd_ethtool_rxvlan_offload(node, intf,op):
    return ethtool_feature_cmd(node, intf,"txvlan", op)

def bsd_ethtool_txvlan_offload(node, intf,op):
    return ethtool_feature_cmd(node, intf,"rxvlan", op)

def bsd_ethtool_hkey_offload(node, intf,op):
    return "echo > /dev/null"

def bsd_ethtool_indirection_table(node, intf,op):
    return "echo > /dev/null"

def bsd_ethtool_rssflow(node, intf,op):
    return "echo > /dev/null"

def bsd_ip_link_filter_mcast(node, intf,op):
    return "echo > /dev/null"

def bsd_ip_link_filter_all_mcast(node, intf,op):
    return "echo > /dev/null"

def bsd_ip_link_filter_broadcast(node, intf,op):
    return "echo > /dev/null"

def bsd_ip_link_filter_promiscuous(node, intf,op):
    return " ".join(["ifconfig", intf,"promisc" if op == "on" else "-promisc"])
