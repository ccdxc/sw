#! /usr/bin/python3

__json_template = {
    "kind"                  : "Tenant",

    "meta": {
        "name"              : "",
        "tenant"            : "",
        "namespace"         : "",
        "generation-id"     : "",
        "creation-time"     : "1970-01-01T00:00:00Z",
        "mod-time"          : "1970-01-01T00:00:00Z"
    },

    "status"                : {}
}

class NetagentTenantObject(self):
    def __init__(self):
        return

def GenerateConfig(params):
    objs = []
    for tid in range(params.num_tenants):
        tenant = NetagentTenantObject()
        objs.append(tenant)
    return objs
