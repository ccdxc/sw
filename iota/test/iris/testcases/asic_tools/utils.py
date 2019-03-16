#! /usr/bin/python3
import iota.harness.api as api

g_path = 'PATH=$PATH:/platform/bin/; LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/platform/lib/:/nic/lib/; export PATH; export LD_LIBRARY_PATH; '

def naples_get(tc):
    nodes = api.GetNaplesHostnames()
    if len(nodes) == 0:
        api.Logger.info("Skipping testcase as there are no naples nodes")
        tc.skip = True
        return api.types.status.IGNORED
    tc.naples_node = nodes[0]
    api.Logger.verbose(tc.naples_node)
    return api.types.status.SUCCESS
