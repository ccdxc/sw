#! /usr/bin/python3

__json_template = { "security-profiles" : [
    {
    "kind"                  : "SecurityProfile",

    "meta": {
        "name"              : "",
        "tenant"            : "",
        "namespace"         : "",
        "creation-time"     : "1970-01-01T00:00:00Z",
        "mod-time"          : "1970-01-01T00:00:00Z"
    },

    "spec": {
        "timeouts": {
          "session-idle": "",
          "tcp": "",
          "tcp-drop": "",
          "tcp-connection-setup": "",
          "tcp-half-close": "",
          "tcp-close": "",
          "drop": "",
          "udp": "",
          "udp-drop": "",
          "icmp": "",
          "icmp-drop": ""
        }
    },
    "status"                : {}
    }
    ]
}
gl_securityprofile_json_template = __json_template
class NetagentSecurityProfileObject:
    def __init__(self):
        return
