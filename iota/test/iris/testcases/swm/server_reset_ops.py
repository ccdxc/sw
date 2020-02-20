#! /usr/bin/python3
import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.ping import ping
from .utils.getcfg import getcfg
from .utils.server import server_reset
from .utils.ncsi_ops import check_set_ncsi
import time

def Setup(tc):
    tc.cfg = getcfg()
    try:
        check_set_ncsi(tc.cfg)
        # Create a Redfish client object
        tc.RF = redfish_client(base_url="https://{}".format(tc.cfg['NCSI_IP']),
                               username=tc.cfg['ILO_UNAME'],
                               password=tc.cfg['ILO_PSWD'])
        # Login with the Redfish client
        tc.RF.login()
    except ServerDownOrUnreachableError:
        api.Logger.error("{} ILO ip not reachable or does not support RedFish".format(tc.cfg['NCSI_IP']))
        return api.types.status.ERROR
    except Exception as e:
        api.Logger.error(str(e))
        return api.types.status.ERROR

    return api.types.status.SUCCESS

def Trigger(tc):
    PING_CNT = 200
    try:
        for _i in range(500):
            print('='*80)
            print(' '*30 + 'Iteration %d' % _i + ' '*30)
            print('='*80)
            if _i % 2 == 0:    
                server_reset(tc.RF, 'ForceRestart')
            else:
                server_reset(tc.RF, 'ForceOff')
                time.sleep(10)
                server_reset(tc.RF, 'On')
            ret = ping(tc.cfg['HOST_IP'], PING_CNT)
            if ret != api.types.status.SUCCESS:
                raise RuntimeError('HOST is not up, after time out')
    except Exception as e:
        api.Logger.error(str(e))
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    try:
        tc.RF.logout()
    except Exception as e:
        api.Logger.error(str(e))
        return api.types.status.ERROR

    return api.types.status.SUCCESS