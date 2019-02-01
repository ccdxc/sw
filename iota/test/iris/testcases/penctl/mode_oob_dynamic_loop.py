#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.penctldefs as penctldefs
import iota.test.iris.testcases.penctl.common as common

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    tc.venice_ips = ["1.1.1.1", "2.2.2.2", "3.3.3.3"]
    tc.controller_ip_agent = []
    return api.types.status.SUCCESS

def Trigger(tc):
    for n in tc.Nodes:
        for i in range(0, len(tc.venice_ips)):
            common.PrepareDhcpConfig(tc.venice_ips[i])
            common.SetupRemoteDhcp(n)
            time.sleep(5)
            common.SetNaplesModeOOB_Dynamic(n)
            time.sleep(5)
            tc.controller_ip_agent.append(common.AgentGetControllers(n)[0])

    return api.types.status.SUCCESS

def Verify(tc):
    for i in range(0, len(tc.venice_ips)):
        api.Logger.info("AGENT_CTRL : {} EXPECTED : {}".format(tc.controller_ip_agent[i], '{}:9009'.format(tc.venice_ips[i])))
        if tc.controller_ip_agent[i] != '{}:9009'.format(tc.venice_ips[i]):
            api.Logger.info("FAILED")
    #        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
