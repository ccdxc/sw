#! /usr/bin/python3
import time
import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.ncsi_ops import set_ncsi_mode
from .utils.ping import ping
from .utils.getcfg import getcfg


def Setup(tc):
    tc.cfg = getcfg()
    try:
        # Create a Redfish client object
        tc.RF = redfish_client(base_url="https://{}".format(tc.cfg['ILO_IP']),
                               username=tc.cfg['ILO_UNAME'],
                               password=tc.cfg['ILO_PSWD'])
        # Login with the Redfish client
        tc.RF.login()
    except ServerDownOrUnreachableError:
        api.Logger.error("{} ILO ip not reachable or does not support RedFish".format(tc.cfg['ILO_IP']))
        return api.types.status.ERROR
    except Exception as e:
        api.Logger.error(str(e))
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def Trigger(tc):
    PING_CNT = 120
    try:
        ret = set_ncsi_mode(tc.RF, tc.cfg['ILO_IP'],
                        tc.cfg['ILO_GW'],
                        tc.cfg['ILO_NETMASK'])
        time.sleep(5)
        if ret is True:
            ret = ping(tc.cfg['ILO_IP'], PING_CNT)
            if ret is not True:
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
