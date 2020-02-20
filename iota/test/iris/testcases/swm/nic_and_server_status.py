#! /usr/bin/python3
import iota.harness.api as api

from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from .utils.getcfg import getcfg
from pprint import pprint
from .utils.ncsi_ops import check_set_ncsi


def get_nics_health(_robj):
    mgr_uri = _robj.root['Systems']['@odata.id']
    mgr_resp = _robj.get(mgr_uri)
    mgr_member_uri = next(iter(mgr_resp.obj['Members']))['@odata.id']
    mgr_member_resp = _robj.get(mgr_member_uri)
    nics_uri = mgr_member_resp.obj['Oem']['Hpe']['Links']['NetworkAdapters']['@odata.id']
    nics_resp = _robj.get(nics_uri)
    nics_status = {}
    for _nic_member in nics_resp.obj['Members']:
        _nic_uri = _nic_member['@odata.id']
        _nic_resp = _robj.get(_nic_uri)
        nics_status[_nic_resp.obj['Name']] = _nic_resp.obj['Status']

    return nics_status


def get_server_health(_robj):
    systems_uri = _robj.root['Systems']['@odata.id']
    systems_resp = _robj.get(systems_uri)
    system_uri = next(iter(systems_resp.obj['Members']))['@odata.id']
    system_obj = _robj.get(system_uri).obj
    status = system_obj['Status']
    
    return status


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
    try:
        api.Logger.info(pprint(get_nics_health(tc.RF)))
        api.Logger.info(pprint(get_server_health(tc.RF)))
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