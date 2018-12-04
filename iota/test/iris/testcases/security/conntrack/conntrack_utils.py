#! /usr/bin/python3

import iota.harness.api as api

def start_nc_server(server, lport):
    cmd_cookie = "nc -l {}". format(lport)
    return cmd_cookie

def start_nc_client(host, srcport, dstport):
    cmd_cookie = "nc {} {} -p {}".format(host.ip_address, dstport, srcport)
    return cmd_cookie
        
def add_command(req, tc, cookie_string, host, cmd_cookie, background=False, naples=False):
    if naples:
        api.Trigger_AddNaplesCommand(req, host.node_name, cmd_cookie)
    else:
        api.Trigger_AddCommand(req, host.node_name, host.workload_name, cmd_cookie, background)
    tc.cmd_cookies[cookie_string] = cmd_cookie

def wrap_around(value, delta):
    if delta >= 0:
        return (value + delta) % (2 ** 32)
    else:
        return ((value + delta) + (2 ** 32)) % (2 ** 32)



