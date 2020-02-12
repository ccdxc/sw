#! /usr/bin/python3
import pdb
import os
import json
import sys

import iota.harness.api as api
import iota.harness.infra.utils.parser as parser
import iota.test.apulu.config.api as config_api

import iota.protos.pygen.topo_svc_pb2 as topo_svc


def __get_route_add_cmd(route, gw):
    route_base_cmd = "ip route add %s via %s"
    route_cmd = route_base_cmd % (route, gw)
    api.Logger.info(route_cmd)
    return route_cmd

def __add_routes():

    vnic_routes = config_api.GetVnicRoutes()
    if not vnic_routes:
        api.Logger.info("No vnic routes to add")
        return api.types.status.SUCCESS
    else:
        req = api.Trigger_CreateExecuteCommandsRequest()
        for vnic_route in vnic_routes:
            for route in vnic_route.routes:
                route_cmd = __get_route_add_cmd(route, vnic_route.gw)
                api.Trigger_AddHostCommand(req, vnic_route.node_name, route_cmd)

    resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Main(step):
    api.Logger.info("Adding route entries")
    return __add_routes()

if __name__ == '__main__':
    Main(None)
