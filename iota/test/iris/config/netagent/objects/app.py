#! /usr/bin/python3

__json_template = { "apps" : [
    {
    "kind"                  : "App",

    "meta": {
        "name"              : "",
        "tenant"            : "",
        "namespace"         : "",
        "creation-time"     : "1970-01-01T00:00:00Z",
        "mod-time"          : "1970-01-01T00:00:00Z"
    },

    "spec": {
        "proto-ports"      : [
        ],
        "alg-type"         : "",
        "alg"             : "",
        "app-idle-timeout"  : "",
    },
    "status"                : {}
    }
    ]
}
gl_app_json_template = __json_template
class NetagentAppObject:
    def __init__(self):
        return
