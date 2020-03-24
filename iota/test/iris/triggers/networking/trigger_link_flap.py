import iota.harness.api as api
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

def Main(tc):
    if not GlobalOptions.regression:
        api.Logger.verbose("Skipping link flap trigger")
        return api.types.status.SUCCESS
    return api.FlapDataPorts(api.GetNaplesHostnames(), num_ports_per_node = 2, down_time=1)
