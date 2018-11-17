#! /usr/bin/python3

__json_template = { "sgpolicies" : [
    {
    "kind"                  : "SGPolicy",

    "meta": {
        "name"              : "",
        "tenant"            : "",
        "namespace"         : "",
        "creation-time"     : "1970-01-01T00:00:00Z",
        "mod-time"          : "1970-01-01T00:00:00Z"
    },

    "spec": {
        "attach-tenant"     : True,
        "policy-rules"      : [
        ]
    },
    "status"                : {}
    }
    ]
}
gl_sg_json_template = __json_template
class NetagentSGObject:
    def __init__(self):
        return
