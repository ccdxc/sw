import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common

def Main(step):
    nodes = api.GetWorkloadNodeHostnames()
    api.RestartNodes(nodes)
    return api.types.status.SUCCESS

