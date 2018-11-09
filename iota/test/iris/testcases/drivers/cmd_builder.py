#! /usr/bin/python3

def ethtool_feature_cmd(intf, feature, on_off):
    return " ".join(["ethtool", "-K",  intf,  feature,  on_off])

def ethtool_ring_size_cmd(intf, ring_type, size):
    return " ".join(["ethtool", "-G",  intf,  ring_type,  size])

def ethtool_queue_size_cmd(intf, queue_type, size):
    return " ".join(["ethtool", "-L",  intf,  queue_type,  size])

def ip_link_pkt_filter_cmd(intf, pkt_filter, on_off):
    return " ".join(["ip", "link", "set", "dev", intf, pkt_filter, on_off])

def ip_link_ucast_mac_filter_cmd(intf, mac):
    return " ".join(["ip", "link", "set", "dev", intf, "address", mac])

def ip_link_mtu_cmd(intf, mtu):
    return " ".join(["ip", "link", "set", "dev", intf, "mtu", mtu])

def ethtool_checksum_cmd(intf, feature, on_off):
    return " ".join(["ethtool", "--offload",  intf,  feature,  on_off])

def ethtool_tx_ring_size(intf, size):
    return ethtool_ring_size_cmd(intf, "tx", size)

def ethtool_rx_ring_size(intf, size):
    return ethtool_ring_size_cmd(intf, "rx", size)

def ethtool_tx_queue_size(intf, size):
    return ethtool_queue_size_cmd(intf, "tx", size)

def ethtool_rx_queue_size(intf, size):
    return ethtool_queue_size_cmd(intf, "rx", size)

def ethtool_tx_checksum(intf, op):
    return ethtool_checksum_cmd(intf, "tx", op)

def ethtool_rx_checksum(intf, op):
    return ethtool_checksum_cmd(intf, "rx", op)

def ethtool_tso_offload(intf, op):
    return ethtool_feature_cmd(intf, "tso", op)

def ethtool_gro_offload(intf, op):
    return ethtool_feature_cmd(intf, "gro", op)

def ethtool_sg_offload(intf, op):
    return ethtool_feature_cmd(intf, "sg", op)

def ethtool_rxvlan_offload(intf, op):
    return ethtool_feature_cmd(intf, "txvlan", op)

def ethtool_txvlan_offload(intf, op):
    return ethtool_feature_cmd(intf, "rxvlan", op)


def ip_link_filter_mcast(intf, op):
    return ip_link_pkt_filter_cmd(intf, "multicast", op)

def ip_link_filter_all_mcast(intf, op):
    return ip_link_pkt_filter_cmd(intf, "allmulticast", op)

def ip_link_filter_broadcast(intf, op):
    return ip_link_pkt_filter_cmd(intf, "broadcast", op)

def ip_link_filter_promiscuous(intf, op):
    return ip_link_pkt_filter_cmd(intf, "promisc", op)

def tcpdump_cmd(intf, filename):
    return " ".join(["tcpdump", "-xxi", intf, "-w",  filename])

def iperf_server_cmd(port=5200, time=300, run_core=None):
    cmd = ["iperf", "-s","-p", str(port)]
    if run_core:
        cmd.extend(["-A", str(run_core)])

    if time:
        cmd.extend(["-t", str(time)])

    return " ".join(cmd)

def iperf_client_cmd(server_ip, port=5200, time=10, pktsize=None, proto='tcp', run_core=None, ipproto='v4'):
    cmd = ["iperf", "-c", str(server_ip), "-p", str(port)]

    if time:
        cmd.extend(["-t", str(time)])

    if run_core:
        cmd.extend(["-A", str(run_core)])

    if proto == 'udp':
        cmd.append('-u')

    if pktsize:
        if proto == 'tcp':
            cmd.extend(["-M", str(pktsize)])
        else:
            cmd.extend(["-l", str(pktsize)])

    if ipproto == 'v6':
        cmd.append("-6")

    return " ".join(cmd)
