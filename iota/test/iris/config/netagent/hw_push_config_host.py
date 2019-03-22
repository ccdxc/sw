#! /usr/bin/python3
import pdb
import os
import json

import iota.harness.api as api
import iota.harness.infra.utils.parser as parser
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.config.netagent.hw_push_config as hw_push_config
import iota.protos.pygen.topo_svc_pb2 as topo_svc


_netagent_cfg_dir = api.GetTopDir() + "/iota/test/iris/config/netagent/cfg/"
_netagent_gen_cfg_dir = api.GetTopDir() + "/iota/test/iris/config/netagent/gen_cfg/"

def __generate_config(cfgMetaFile, cfgOutputDir):
    def __generate_heimdall_template():
        tmpl_file = _netagent_cfg_dir + "/heimdall.tmpl.json"
        tmpl_cfg = json.load(open(tmpl_file))
        tmpl_cfg["vlan-offset"] = api.Testbed_AllocateVlan()
        tmpl_output = cfgOutputDir +  "/heimdall.tmpl.json"
        with open(tmpl_output, 'w') as outfile:
            json.dump(tmpl_cfg, outfile)
        return tmpl_output

    os.system("rm -rf " + cfgOutputDir)
    os.mkdir(cfgOutputDir)
    tmpl_file = __generate_heimdall_template()
    heimdallExec = api.GetTopDir() + '/nic/build/x86_64/iris/bin/heimdall'
    cmd = heimdallExec + " gen -f " + cfgMetaFile + " -t " + tmpl_file +  " -o " + cfgOutputDir +  " -u node1 --remote-uuid node2"
    ret = os.system(cmd)
    if ret != 0:
        api.Logger.error("Failed to generate heimdall config")
        return api.types.status.FAILURE
    os.system("rm -f " + tmpl_file)

    return api.types.status.SUCCESS


def Main(args):
    #time.sleep(120)

    agent_nodes = api.GetNaplesHostnames()
    netagent_api.Init(agent_nodes, hw = True)

    #For now this is run for scale only, disable hal debug logs
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    for node in agent_nodes:
        api.Trigger_AddNaplesCommand(req, node, "/nic/bin/halctl debug trace --level error")

    api.Trigger(req)

    cfgMetaFile = _netagent_cfg_dir + "/" + args.spec
    cfgOutputDir =  _netagent_gen_cfg_dir
    ret = __generate_config(cfgMetaFile, cfgOutputDir)

    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    netagent_api.ReadConfigs(cfgOutputDir)
    #Delete path is not stable yet
    #netagent_api.DeleteBaseConfig()
    netagent_api.PushBaseConfig()


    for node in api.GetWorkloadNodeHostnames():
        if api.GetWorkloadTypeForNode(node) == topo_svc.WorkloadType.Value('WORKLOAD_TYPE_BARE_METAL_MAC_VLAN_ENCAP'):
            req = api.Trigger_CreateExecuteCommandsRequest()
            cmd = "echo 2 >/proc/sys/net/ipv4/conf/all/rp_filter"
            api.Trigger_AddHostCommand(req, node, cmd)
            api.Trigger(req)

    hw_push_config.AddWorkloads()
    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
