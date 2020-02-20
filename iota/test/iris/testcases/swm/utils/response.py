import json
import traceback

import iota.harness.api as api

def validate_resp(resp):
    if resp.status == 400:
        try:
            api.Logger.error(json.dumps(resp.obj['error']\
                                ['@Message.ExtendedInfo'], indent=4, sort_keys=True))
        except:
            api.Logger.error(traceback.format_exc())
            return api.types.status.FAILURE
    elif resp.status != 200:
        api.Logger("An http response of \'%s\' was returned.\n" % resp.status)
        return api.types.status.FAILURE
    else:
        return api.types.status.SUCCESS
