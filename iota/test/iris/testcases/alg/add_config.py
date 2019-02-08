import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as netagent_cfg_api
from iota.test.iris.testcases.alg.alg_utils import *

def Setup(tc):
    return api.types.status.SUCCESS


def Trigger(tc):
    netagent_cfg_api.ReadConfigs("test/iris/topologies/2Naples_VM")
    netagent_cfg_api.AddSgPolicies()
    update_sgpolicy(None, True)
    return api.types.status.SUCCESS


def Verify(tc):
    result = api.types.status.SUCCESS
    return result


def Teardown(tc):
    return api.types.status.SUCCESS
