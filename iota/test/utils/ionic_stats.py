import iota.harness.api as api
import iota.test.utils.naples_host as host

# In netstat output for FreeBSD, first one is received bad checksum counter.
NETSTAT_BSD_ICMP_BAD_CHECKSUM = 'sudo netstat -s -p icmp | grep "with bad checksum" | cut -d " " -f 1| tr -d " " '
NETSTAT_BSD_TCP_BAD_CHECKSUM = 'sudo netstat -s -p tcp | grep "discarded for bad checksum" | cut -d " " -f 1 | tr -d " " '
NETSTAT_BSD_UDP_BAD_CHECKSUM = 'sudo netstat -s -p udp | grep "with bad checksum" | cut -d " " -f 1| tr -d " " '

NETSTAT_LINUX_ICMP_BAD_CHECKSUM = 'sudo netstat -s -w | grep InCsumErrors | cut -d ":" -f 2 | tr -d " " '
NETSTAT_LINUX_TCP_BAD_CHECKSUM = 'sudo netstat -s -t | grep InCsumErrors | cut -d ":" -f 2 | tr -d " " '
NETSTAT_LINUX_UDP_BAD_CHECKSUM = 'sudo netstat -s -u | grep InCsumErrors | cut -d ":" -f 2 | tr -d " " '

def getNetstatBadCsum(node, proto):
    if api.GetNodeOs(node.node_name) != host.OS_TYPE_BSD and api.GetNodeOs(node.node_name) != host.OS_TYPE_LINUX :
        return 0

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    if api.GetNodeOs(node.node_name) == host.OS_TYPE_BSD:
        if proto == 'tcp':
            api.Trigger_AddHostCommand(req, node.node_name, NETSTAT_BSD_TCP_BAD_CHECKSUM)
        elif proto == 'udp':
            api.Trigger_AddHostCommand(req, node.node_name, NETSTAT_BSD_UDP_BAD_CHECKSUM)
        else:
            api.Trigger_AddHostCommand(req, node.node_name, NETSTAT_BSD_ICMP_BAD_CHECKSUM)
    else:
        if proto == 'tcp':
            api.Trigger_AddHostCommand(req, node.node_name, NETSTAT_LINUX_TCP_BAD_CHECKSUM)
        elif proto == 'udp':
            api.Trigger_AddHostCommand(req, node.node_name, NETSTAT_LINUX_UDP_BAD_CHECKSUM)
        else:
            api.Trigger_AddHostCommand(req, node.node_name, NETSTAT_LINUX_ICMP_BAD_CHECKSUM)

    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error(
            "Failed to start netstat on %s" % (node))
        return -1

    for cmd in resp.commands:
        if cmd.exit_code != 0:
            if cmd.stdout is not None:
                api.Logger.error("Received bad checksum counter is non-zero(%s)"
                                 % (cmd.stdout))
                api.PrintCommandResults(cmd)
                return -1

    if cmd.stdout == "":
        return -1
    else:
        return int(cmd.stdout)

def getBsdStats(node, intf, pattern):
    stats_map = []
    
    cmd = 'sysctl dev.' + host.GetNaplesSysctl(intf) + ' | grep -e ' + pattern + ' | cut -d ":" -f 2'
    
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    #api.Logger.info("Getting sysctl map for: %s on host: %s intf: %s" 
    #                %(cmd, node.node_name, intf))
    api.Trigger_AddHostCommand(req, node.node_name, cmd)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to run: %s on host: %s intf: %s" 
                         %(cmd, node.node_name, intf))
        return None

    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error(
            "Failed to run: %s for host: %s, stderr: %s"
            %(cmd, node.node_name, cmd.stderr))
        api.PrintCommandResults(cmd)
        return None

    if cmd.stdout == "":
        api.Logger.error("Output is empty for: %s on host: %s intf: %s" 
                         %(cmd, node.node_name, intf))
        api.PrintCommandResults(cmd)
        return None
    
    stats_map = cmd.stdout.splitlines()
    stats_map = list(map(int,stats_map))
    
    return stats_map

def getLinuxStats(node, intf, pat1):
    stats_map = []
    
    cmd = 'ethtool -S ' + intf + ' | grep -e ' + pat1 + ' | cut -d ":" -f 2'
    
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    #api.Logger.info("Getting stats for: %s on host: %s intf: %s" 
    #                %(cmd, node.node_name, intf))
    api.Trigger_AddHostCommand(req, node.node_name, cmd)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to run: %s on host: %s intf: %s" 
                         %(cmd, node.node_name, intf))
        return None

    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error(
            "Failed to run: %s for host: %s, stderr: %s"
            %(cmd, node.node_name, cmd.stderr))
        api.PrintCommandResults(cmd)
        return None

    if cmd.stdout == "":
        api.Logger.error("Output is empty for: %s on host: %s intf: %s" 
                         %(cmd, node.node_name, intf))
        api.PrintCommandResults(cmd)
        return None
    
    stats_map = cmd.stdout.splitlines()
    stats_map = list(map(int,stats_map))
    
    return stats_map

def getTxNoCsumStats(node, intf):
    if not node.IsNaples():
        return None
    
    if api.GetNodeOs(node.node_name) == host.OS_TYPE_BSD:
        return getBsdStats(node, intf, 'txq[0-9]*.\.no_csum')
    elif api.GetNodeOs(node.node_name) == host.OS_TYPE_LINUX:
        return getLinuxStats(node, intf, 'tx_no_csum\|tx.*csum_none')
    
    return None

def getTxCsumStats(node, intf):
    if not node.IsNaples():
        return None
    
    if api.GetNodeOs(node.node_name) == host.OS_TYPE_BSD:
        return getBsdStats(node, intf, 'txq[0-9]*.\.csum')
    elif api.GetNodeOs(node.node_name) == host.OS_TYPE_LINUX:
        return getLinuxStats(node, intf, 'tx_csum')
    
    return None

def getRxL4CsumGoodStats(node, intf):
    if not node.IsNaples():
        return None
    
    if api.GetNodeOs(node.node_name) == host.OS_TYPE_BSD:
        return getBsdStats(node, intf, 'rxq[0-9]*.\.csum_l4_ok')
    elif api.GetNodeOs(node.node_name) == host.OS_TYPE_LINUX:
        return getLinuxStats(node, intf, 'rx_[0-9]*_csum_complete')

    return None

def getRxL4CsumBadStats(node, intf):
    if not node.IsNaples():
        return None
    
    if api.GetNodeOs(node.node_name) == host.OS_TYPE_BSD:
        return getBsdStats(node, intf, 'rxq[0-9]*.\.csum_l4_bad')
    elif api.GetNodeOs(node.node_name) == host.OS_TYPE_LINUX:
        return getLinuxStats(node, intf, 'rx_[0-9]*_csum_error')

    return None

def getTSOIPv4Stats(node, intf):
    if not node.IsNaples():
        return None
    
    if api.GetNodeOs(node.node_name) == host.OS_TYPE_BSD:
        return getBsdStats(node, intf, 'txq[0-9]*.\.tso_ipv4')
    elif api.GetNodeOs(node.node_name) == host.OS_TYPE_LINUX:
        return getLinuxStats(node, intf, 'tx_tso') 
           
    return None

def getTSOIPv6Stats(node, intf):
    if not node.IsNaples():
        return None
    
    if api.GetNodeOs(node.node_name) == host.OS_TYPE_BSD:
        return getBsdStats(node, intf, 'txq[0-9]*.\.tso_ipv6')
    elif api.GetNodeOs(node.node_name) == host.OS_TYPE_LINUX:
        return getLinuxStats(node, intf, 'tx_tso')
    
    return None

def getLROStats(node, intf):
    if not node.IsNaples():
        return None

    if api.GetNodeOs(node.node_name) == host.OS_TYPE_BSD:
        return getBsdStats(node, intf, 'rxq[0-9]*.\.lro_queue')

    return None

