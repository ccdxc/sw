from .response import validate_resp
from .ilo_ops import reset_ilo
import iota.harness.api as api

def get_nic_mode(_redfishobj):
    ethernet_data = {}

    managers_uri = _redfishobj.root['Managers']['@odata.id']
    managers_response = _redfishobj.get(managers_uri)
    managers_members_uri = next(iter(managers_response.obj['Members']))['@odata.id']
    managers_members_response = _redfishobj.get(managers_members_uri)
    manager_ethernet_interfaces = managers_members_response.obj['EthernetInterfaces']\
                                                                                ['@odata.id']
    manager_ethernet_interfaces_response = _redfishobj.get(manager_ethernet_interfaces)
    manager_ethernet_interfaces_members = manager_ethernet_interfaces_response.\
                                                        obj['Members']
    for _member in manager_ethernet_interfaces_members:
        _tmp = _redfishobj.get(_member['@odata.id']).obj
        ethernet_data[_member['@odata.id']] = _tmp

    for ethernet in ethernet_data:
        eth_obj = ethernet_data[ethernet]
    
        if eth_obj['InterfaceEnabled'] is True:
            if eth_obj['Oem']['Hpe']['InterfaceType'] == "Dedicated":
                return "dedicated"
            elif eth_obj['Oem']['Hpe']['InterfaceType'] == "Shared":
                return "ncsi"
            else:
                return eth_obj['Oem']['Hpe']['InterfaceType']
    return None


def get_nic_obj(_redfishobj, nic_type="dedicated"):
    ethernet_data = {}

    managers_uri = _redfishobj.root['Managers']['@odata.id']
    managers_response = _redfishobj.get(managers_uri)
    managers_members_uri = next(iter(managers_response.obj['Members']))['@odata.id']
    managers_members_response = _redfishobj.get(managers_members_uri)
    manager_ethernet_interfaces = managers_members_response.obj['EthernetInterfaces']\
                                                                                ['@odata.id']
    manager_ethernet_interfaces_response = _redfishobj.get(manager_ethernet_interfaces)
    manager_ethernet_interfaces_members = manager_ethernet_interfaces_response.\
                                                        obj['Members']
    for _member in manager_ethernet_interfaces_members:
        _tmp = _redfishobj.get(_member['@odata.id']).obj
        ethernet_data[_member['@odata.id']] = _tmp

    for ethernet in ethernet_data:
        eth_obj = ethernet_data[ethernet]
    
        if eth_obj['Oem']['Hpe']['InterfaceType'] == "Dedicated"\
                and nic_type == "dedicated":
            return eth_obj
        elif eth_obj['Oem']['Hpe']['InterfaceType'] == "Shared"\
                and nic_type == "ncsi":
            return eth_obj
    return None

def enable_vlan_mode(_redfishobj, nic_obj, vlan_id=1):
    body = {
        "VLAN":   {
            "VLANEnable": True,
            "VLANId": vlan_id
        }
    }
    resp = _redfishobj.patch(nic_obj['@odata.id'], body=body)
    ret = validate_resp(resp)
    if ret == api.types.status.SUCCESS:
        reset_ilo(_redfishobj)
    else:
        return api.types.status.FAILURE
    
    return api.types.status.SUCCESS

def disable_vlan_mode(_redfishobj, nic_obj):
    body = {
        "VLAN":   {
            "VLANEnable": False,
            "VLANId": null
        }
    }
    resp = _redfishobj.patch(nic_obj['@odata.id'], body=body)
    ret = validate_resp(resp)
    if ret == api.types.status.SUCCESS:
        reset_ilo(_redfishobj)
    else:
        return api.types.status.FAILURE
    
    return api.types.status.SUCCESS