import sys
import json
import time
import traceback

from .ilo_ops import reset_ilo
from .nic_ops import get_nic_obj
from .response import validate_resp
from iota.harness.infra.redfish import redfish_client
from iota.harness.infra.redfish.rest.v1 import ServerDownOrUnreachableError
from iota.harness.infra.redfish.rest.v1 import InvalidCredentialsError
from .ping import ping
import iota.harness.api as api
from .common import get_redfish_obj

def set_ncsi_mode(RF, ip=None, gateway=None, netmask=None, mode="dhcp"):
    eth_obj = get_nic_obj(RF, nic_type="ncsi")
    if eth_obj is None:
        print("No ncsi interface found, exiting...")
        return api.types.status.FAILURE

    if mode == "static":
        body = {
            "InterfaceEnabled": True,
            "DHCPv4":   {
                "DHCPEnabled": False,
            },
            "IPv4Addresses":
            [
                {
                    "Address": ip,
                    "Gateway": gateway,
                    "SubnetMask": netmask,
                }
            ],
        }
    elif mode == "dhcp":
        body = {
            "InterfaceEnabled": True,
            "DHCPv4":   {
                "DHCPEnabled": True,
                "UseDNSServers": True,
                "UseDomainName": True,
                "UseGateway": True,
                "UseNTPServers": True,
                "UseStaticRoutes": True
            },
        }
    else:
        print("INVALID mode: {}".format(mode))
        return api.types.status.FAILURE

    resp = RF.patch(eth_obj['@odata.id'], body=body)
    ret = validate_resp(resp)
    if ret == api.types.status.SUCCESS:
        reset_ilo(RF)
    else:
        return api.types.status.FAILURE
    
    return api.types.status.SUCCESS

def switch_ncsi_ports(RF):
    eth_obj = get_nic_obj(RF, nic_type="ncsi")
    if eth_obj is None:
        api.Logger.error("No ncsi interface found, exiting...")
        return api.types.status.FAILURE

    curr_port = eth_obj.Oem.Hpe.SharedNetworkPortOptions['Port']
    new_port = 1 if curr_port == 2 else 2
    api.Logger.info("Current NCSI port %d new port %d" % (curr_port, new_port))
    body = {
        'Oem': {
            'Hpe': {
                'SharedNetworkPortOptions': {
                    'Port': new_port
                }
            }
        }
    }
    resp = RF.patch(eth_obj['@odata.id'], body=body)
    ret = validate_resp(resp)
    if ret == api.types.status.SUCCESS:
        reset_ilo(RF)
    else:
        return api.types.status.FAILURE
    
    return api.types.status.SUCCESS

def check_set_ncsi(cimc_info):
    ret = ping(cimc_info.GetNcsiIp(), 4)
    if ret != api.types.status.SUCCESS:
        RF = get_redfish_obj(cimc_info, mode="dedicated")
        ret = set_ncsi_mode(RF, mode="dhcp")
        if ret != api.types.status.SUCCESS:
            raise RuntimeError('Setting ILO in NCSI mode failed')
        time.sleep(5)
        ret = ping(cimc_info.GetNcsiIp(), 90)
        if ret != api.types.status.SUCCESS:
            raise RuntimeError('ILO in NCSI mode is not reachable')
    
def check_ncsi_conn(cimc_info):
    try:
        # Create a Redfish client object
        RF = get_redfish_obj(cimc_info, mode="ncsi")
        RF.logout()
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE
    
    return api.types.status.SUCCESS
