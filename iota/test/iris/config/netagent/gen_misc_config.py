#! /usr/bin/python3
import pdb
import os

import iota.harness.api as api

_netagent_cfg_dir = api.GetTopDir() + "/iota/test/iris/config/netagent/cfg/"
_netagent_gen_cfg_dir = api.GetTopDir() + "/iota/test/iris/config/netagent/gen_cfg/"
_flommon_cfg_script = api.GetTopDir() + "/iota/test/iris/config/netagent/flowmongenpolicy.py"
_flowmirror_cfg_script = api.GetTopDir() + "/iota/test/iris/config/netagent/mirrorgenpolicy.py"


def __generate_flmon_mirroring_config():
    #Generate flowmon and mirror policy too
    #This also should come from heimdall
    flowMonGenExec = _flommon_cfg_script + " --base-cfg " +  _netagent_gen_cfg_dir
    ret = os.system(flowMonGenExec)
    if ret != 0:
        api.Logger.error("Failed to generate flowmon config")
        return api.types.status.FAILURE

    flowMirrirGenExec = _flowmirror_cfg_script + " --base-cfg " +  _netagent_gen_cfg_dir
    ret = os.system(flowMirrirGenExec)
    if ret != 0:
        api.Logger.error("Failed to generate flow mirror config")
        return api.types.status.FAILURE


def Main(args):
    ret = __generate_flmon_mirroring_config()

    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
