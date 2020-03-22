import iota.harness.api as api

def GetBondWorkloadPair(node_name=None):
    pairs = []
    for node in api.GetNaplesNodes():
        if node_name and node_name == node.Name():
            continue
        if node.GetBondIp() == None:
            api.Logger.info("Bond IP not configured on %s, Skipping..."%(node.Name()))
            continue
        for wl in api.GetWorkloads():
            if node.Name() == wl.node_name:
                continue
            if wl.uplink_vlan != 0:
                continue
            pairs.append([node, wl])
    return pairs
