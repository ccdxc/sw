#! /usr/bin/python3
import grpc

import iota.protos.pygen.types_pb2 as types
import iota.protos.pygen.cfg_svc_pb2 as cfg_svc
import iota.protos.pygen.topo_svc_pb2 as topo_svc

from iota.harness.infra.utils.logger import Logger
from iota.harness.infra.glopts import GlobalOptions

IotaSvcChannel = None
def Init():
    server = 'localhost:' + str(GlobalOptions.svcport)
    Logger.info("Creating GRPC Channel to IOTA Service %s" % server)
    IotaSvcChannel = grpc.insecure_channel(server)
    Logger.info("Waiting for IOTA Service to be UP")
    grpc.channel_ready_future(IotaSvcChannel).result()
    Logger.info("Connected to IOTA Service")
    return


