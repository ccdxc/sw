import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as agent_api
from iota.test.iris.testcases.alg.alg_utils import *

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    nwsec_objs = agent_api.QueryConfigs(kind="NetworkSecurityPolicy")
    agent_api.DeleteConfigObjects(nwsec_objs)
    agent_api.RemoveConfigObjects(nwsec_objs)

    nwsec_json = api.GetTopologyDirectory() + "/" + "sgpolicy.json"
    nwsec_objs = agent_api.AddOneConfig(nwsec_json)
    ret = agent_api.PushConfigObjects(nwsec_objs)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Failed to push nwsec policy")
        return ret
    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
