import sys
import json
import time

import iota.harness.api as api
from .ilo_ops import reset_ilo
from .nic_ops import get_nic_obj
from .response import validate_resp
from iota.harness.infra.redfish import redfish_client
from .common import get_redfish_obj
from .ping import ping

def set_dedicated_mode(_redfishobj, ip=None, gateway=None, netmask=None, mode="dhcp"):
    ret = False
    eth_obj = get_nic_obj(_redfishobj, nic_type="dedicated")
    if eth_obj is None:
        print("No dedicated interface found, exiting...")
        return False

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
                "UseDNSServers": False,
                "UseDomainName": False,
                "UseGateway": False,
                "UseNTPServers": False,
                "UseStaticRoutes": False
            },
        }
    else:
        print("INVALID mode: {}".format(mode))
        return False
            
    resp = _redfishobj.patch(eth_obj['@odata.id'], body=body)
    ret = validate_resp(resp)
    if ret == api.types.status.SUCCESS:
        reset_ilo(_redfishobj)
    else:
        return api.types.status.FAILURE
    
    return api.types.status.SUCCESS

def check_set_dedicated_mode(cimc_info):
    ret = ping(cimc_info.GetIp(), 4)
    if ret != api.types.status.SUCCESS:
        RF = get_redfish_obj(cimc_info, mode="ncsi")
        ret = set_dedicated_mode(RF, mode="dhcp")
        if ret != api.types.status.SUCCESS:
            raise RuntimeError('Setting ILO in dedicated mode failed')
        time.sleep(5)
        ret = ping(cimc_info.GetIp(), 90)
        if ret != api.types.status.SUCCESS:
            raise RuntimeError('ILO in dedicated mode is not reachable')