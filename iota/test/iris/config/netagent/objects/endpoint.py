#! /usr/bin/python3
import iota.harness.api as api

gl_json_template = {
    "kind"                  : "Endpoint",
    "meta": {
        "name"              : "",
        "tenant"            : "",
        "namespace"         : "",
        "generation-id"     : "",
        "creation-time"     : "1970-01-01T00:00:00Z",
        "mod-time"          : "1970-01-01T00:00:00Z"
    },

    "spec": {
        "network-name"      : "",
        "interface-type"    : "lif",
        "interface"         : "",
        "ipv4-addresses"    : [""],
        "mac-address"       : "",
        "useg-vlan"         : 0
    },

    "status"                : {}
}

class NetagentEndpointObject(self):
    def __init__(self):
        global gl_json_template
        self.__obj = api.parser.JsonParse(gl_json_template)
        return
