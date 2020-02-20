from .dedicated_mode_ops import check_set_dedicated_mode
import iota.harness.api as api
import traceback

def recover_ilo(cimc_info, node_name):
    try:
        check_set_dedicated_mode(cimc_info)
    except:
        api.Logger.error(traceback.format_exc())
        api.Logger.info("Recovery by apc power cycle")
        ret = api.RestartNodes([node_name], 'apc')
        if ret != api.types.status.SUCCESS:
            api.Logger.info("APC power cycle failed")
            return api.types.status.FAILURE
        api.Logger.info("APC power cycle done")
        check_set_dedicated_mode(cimc_info)