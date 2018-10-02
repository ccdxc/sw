#! /usr/bin/python3
import grpc

import iota.protos.pygen.types_pb2 as types
import iota.protos.pygen.cfg_svc_pb2 as cfg_svc
import iota.protos.pygen.topo_svc_pb2 as topo_svc
import iota.protos.pygen.topo_svc_pb2_grpc as topo_svc_grpc

import iota.harness.infra.utils.utils as utils

from iota.harness.infra.utils.logger import Logger
from iota.harness.infra.glopts import GlobalOptions

IotaSvcChannel = None
TopoSvcStub = None
def Init():
    server = 'localhost:' + str(GlobalOptions.svcport)
    Logger.info("Creating GRPC Channel to IOTA Service %s" % server)
    IotaSvcChannel = grpc.insecure_channel(server)
    Logger.debug("Waiting for IOTA Service to be UP")
    grpc.channel_ready_future(IotaSvcChannel).result()
    Logger.info("Connected to IOTA Service")

    global TopoSvcStub
    TopoSvcStub = topo_svc.TopologyApiStub(IotaSvcChannel)
    return

def __rpc(req, rpcfn):
    utils.LogMessageContents("Request", req, Logger.debug)
    resp = rpcfn(req)
    utils.LogMessageContents("Response", req, Logger.debug)
    return resp

def CleanupTestbed(req):
    global TopoSvcStub
    Logger.info("Cleaning up Testbed:")
    return __rpc(req, TopoSvcStub.CleanUpTestBed)

def InitTestbed(req):
    global TopoSvcStub
    Logger.info("Initializing Testbed:")
    return __rpc(req, TopoSvcStub.InitTestBed)

def AddNodes(req):
    global TopoSvcStub
    Logger.info("Add Nodes:")
    return __rpc(req, TopoSvcStub.AddNodes)
