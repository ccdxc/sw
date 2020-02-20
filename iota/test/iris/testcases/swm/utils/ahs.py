import sys
import logging
import json
import os

import iota.harness.api as api
from .response import validate_resp

download_types =\
    {
        "ALL": "AHSLocation",
        "DAY": "InfoSight",
        "WEEK": "RecentWeek"
    }

def download_ahs(_redfishobj, dtype="DAY", file_path=None):
    if dtype not in download_types.keys():
        dtype="DAY"
    managers_uri = _redfishobj.root['Managers']['@odata.id']
    managers_response = _redfishobj.get(managers_uri)
    managers_members_uri = next(iter(managers_response.obj['Members']))['@odata.id']
    managers_members_response = _redfishobj.get(managers_members_uri)
    ahs_uri = managers_members_response.obj.Oem.Hpe.Links['ActiveHealthSystem']['@odata.id']
    ahs_response = _redfishobj.get(ahs_uri)
    ahslink = ahs_response.obj['Links'][download_types[dtype]]['extref']
    ahs_resp = _redfishobj.get(ahslink)
    if validate_resp(ahs_resp) == api.types.status.SUCCESS:
        if file_path is None:
            file_path = os.devnull
        with open(file_path, 'wb') as ahsout:
            ahsout.write(ahs_resp.read)
            ahsout.close()
            api.Logger.info("AHS Data saved successfully as: \'%s\'" % file_path)
    else:
        raise RuntimeError("Failed to get AHS data")