#! /usr/bin/python3
import time
import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.dedicated_mode_ops import set_dedicated_mode
from .utils.ping import ping
from .utils.getcfg import getcfg
from .utils.ncsi_ops import check_set_ncsi

def Setup(tc):
    test_node = api.GetNodes()[0]
    tc.test_node = test_node
    try:
        #check_set_ncsi(tc.cfg)
        # Create a Redfish client object
        (cimc_username, cimc_password) = test_node.GetCimcCredentials()
        tc.RF = redfish_client(base_url="https://%s" % (test_node.GetCimcIP()),
                               username=cimc_username,
                               password=cimc_password)
        # Login with the Redfish client
        tc.RF.login()
    except ServerDownOrUnreachableError:
        api.Logger.error("%s ILO ip not reachable or does not support RedFish"  
                         % test_node.GetCimcIP())
        return api.types.status.ERROR
    except Exception as e:
        api.Logger.error(str(e))
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def Trigger(tc):
    max_pings = int(getattr(tc.args, "max_pings", 60))
    try:
        ret = set_dedicated_mode(tc.RF)
        time.sleep(5)
        if ret != api.types.status.SUCCESS:
            ret = ping(tc.cfg['ILO_IP'], max_pings)
            if ret != api.types.status.SUCCESS:
                tc.RF.logout()
            else:
                raise RuntimeError('Unable to ping ILO, Port Switch fail')
        else:
            raise RuntimeError('Port switch config failed')
    except Exception as e:
        api.Logger.error(str(e))
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
