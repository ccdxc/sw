import json
import sys

from .response import validate_resp
import iota.harness.api as api

def get_vmedia_obj(_redfishobj):
    managers_uri = _redfishobj.root['Managers']['@odata.id']
    managers_response = _redfishobj.get(managers_uri)
    managers_members_uri = next(iter(managers_response.obj['Members']))['@odata.id']
    managers_members_response = _redfishobj.get(managers_members_uri)
    manager_vmedia_uri = managers_members_response.obj['VirtualMedia']['@odata.id']
    manager_vmedia_response = _redfishobj.get(manager_vmedia_uri)
    if manager_vmedia_response.obj['Members@odata.count'] == 0:
        print("No vmedia available, exiting")
        return None
    for vmedia_member in manager_vmedia_response.obj['Members']:
        vmedia_data = _redfishobj.get(vmedia_member['@odata.id']).obj
        if "CD" in vmedia_data['MediaTypes'] or "DVD" in vmedia_data['MediaTypes']:
            return vmedia_data

    print("No CD/DVD vmedia slot found, exiting...")
    return None


def eject_vmedia(_redfishobj):
    vmedia_data = get_vmedia_obj(_redfishobj)
    if vmedia_data is None:
        return api.types.status.FAILURE
    if vmedia_data['Inserted'] is False:
        print("No Vmedia present, exiting..")
        return api.types.status.SUCCESS
    vmedia_eject_uri = vmedia_data['Actions']['#VirtualMedia.EjectMedia']['target']
    body = {}
    resp = _redfishobj.post(vmedia_eject_uri, body=body)
    return validate_resp(resp)


def mount_vmedia(_redfishobj, vmedia_path):
    vmedia_data = get_vmedia_obj(_redfishobj)
    if vmedia_data is None:
        return False
    vmedia_insert_uri = vmedia_data['Actions']['#VirtualMedia.InsertMedia']['target']
    body = {'Image': vmedia_path}
    resp = _redfishobj.post(vmedia_insert_uri, body=body)
    return validate_resp(resp)
        