#! /usr/bin/python3
import pdb
import os
import json

import iota.harness.api as api
import iota.harness.infra.utils.parser as parser
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.config.workload.api as wl_api
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
    heimdallExec = api.GetTopDir() + '/iota/bin/heimdall'
    cmd = heimdallExec + " gen -f " + cfgMetaFile + " -t " + tmpl_file +  " -o " + cfgOutputDir +  " -u node1 --remote-uuid node2"
    api.Logger.info("Generating config %s" % cmd)
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
        api.Trigger_AddNaplesCommand(req, node, "touch /data/no_watchdog")

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
            #cmd = "echo 2 >/proc/sys/net/ipv4/conf/all/rp_filter"
            #api.Trigger_AddHostCommand(req, node, cmd)
            cmd = "sudo sysctl -w net.core.netdev_max_backlog=100000"
            api.Trigger_AddHostCommand(req, node, cmd)
            cmd = "sudo sysctl -w net.core.somaxconn=65535"
            api.Trigger_AddHostCommand(req, node, cmd)
            cmd = "sudo sysctl -w net.ipv4.tcp_max_syn_backlog=100000"
            api.Trigger_AddHostCommand(req, node, cmd)
            cmd = "sudo sysctl -w net.nf_conntrack_max=300000"
            api.Trigger_AddHostCommand(req, node, cmd)
            cmd = "echo 'net.core.rmem_max=16777216' >> /etc/sysctl.conf"
            api.Trigger_AddHostCommand(req, node, cmd)
            cmd = "echo 'net.core.wmem_max=16777216' >> /etc/sysctl.conf"
            api.Trigger_AddHostCommand(req, node, cmd)
            cmd = "echo 'net.ipv4.tcp_no_metrics_save = 1' >> /etc/sysctl.conf"
            api.Trigger_AddHostCommand(req, node, cmd)

            cmd = "echo 'net.ipv4.ip_local_port_range = 15000 61000' >> /etc/sysctl.conf"
            api.Trigger_AddHostCommand(req, node, cmd)

            cmd = "echo 'net.ipv4.tcp_fin_timeout= 30' >> /etc/sysctl.conf"
            api.Trigger_AddHostCommand(req, node, cmd)

            cmd = "echo 'net.ipv4.tcp_rmem = 4096 4096 16777216' >> /etc/sysctl.conf"
            api.Trigger_AddHostCommand(req, node, cmd)
            cmd = "echo 'net.ipv4.tcp_wmem = 4096 4096 16777216' >> /etc/sysctl.conf"
            api.Trigger_AddHostCommand(req, node, cmd)

            cmd = "echo 1048576 | sudo tee /proc/sys/fs/nr_open"
            api.Trigger_AddHostCommand(req, node, cmd)
            cmd = "echo 1048576 | sudo tee /proc/sys/fs/file-max"
            api.Trigger_AddHostCommand(req, node, cmd)


            resp = api.Trigger(req)
            for cmd in resp.commands:
                if cmd.exit_code != 0 :
                    assert(0)

    wl_api.AddWorkloads()
    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
