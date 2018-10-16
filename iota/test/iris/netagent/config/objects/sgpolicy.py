#! /usr/bin/python3

__json_template = {
    "kind"                  : "SGPolicy",

    "meta": {
        "name"              : "",
        "tenant"            : "",
        "namespace"         : "",
        "generation-id"     : "",
        "creation-time"     : "1970-01-01T00:00:00Z",
        "mod-time"          : "1970-01-01T00:00:00Z"
    },

    "spec": {
        "attach-tenant"     : "",
        "policy_rules"      : [
            {
                "action"    : "PERMIT"
            }
        ]
    },
    "status"                : {}
}

class NetagentEndpointObject(self):
    def __init__(self):
        return
