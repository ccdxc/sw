#! /usr/bin/python3
import pdb
import os
import json
from collections import defaultdict

import iota.test.apollo.config.init as init

#Following come from dol/infra
import infra.common.defs as defs
import infra.common.utils as utils
import infra.common.glopts as glopts
class GlobalOptions: pass
glopts.GlobalOptions = GlobalOptions()
glopts.GlobalOptions.debug = True
glopts.GlobalOptions.verbose = True
import infra.common.parser as parser
import infra.common.timeprofiler as timeprofiler

import iota.test.apollo.config.agent.api as agentapi
import iota.test.apollo.config.resmgr as resmgr
import iota.test.apollo.config.agent.api as agentapi

#import apollo.config.objects.batch as batch
import iota.test.apollo.config.objects.device as device
import iota.test.apollo.config.objects.vpc as vpc
import iota.test.apollo.config.objects.route as route
import iota.test.apollo.config.objects.policy as policy
import iota.test.apollo.config.objects.subnet as subnet
import iota.test.apollo.config.objects.vnic as vnic
import iota.test.apollo.config.objects.tunnel as tunnel
#import apollo.config.objects.mirror as mirror

import iota.harness.api as api
from iota.test.apollo.config.store import Store
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

vpcClient = None

def __generate(topospec):
    # Generate Batch Object
    #batch.client.GenerateObjects(topospec)


    # Generate Device Configuration
    device.client.GenerateObjects(topospec)



    devices = Store.GetDevices()


    #Setup clients for all devices
    vpc.NewVpcObjectClient(devices)
    policy.NewPolicyObjectClient(devices)
    route.NewRouteObjectClient(devices)
    subnet.NewSubnetObjectClient(devices)
    vnic.NewVnicObjectClient(devices)


    # Generate Mirror session configuration
    #mirror.client.GenerateObjects(topospec)

    # Generate VPC configuration
    vpc.client.GenerateObjects(topospec)
    return

def __create():
    # Start the Batch
    #Batch client has to be started to all naples
    #batch.client.Start(c)


    device.client.Start()

    # Create Device Object
    device.client.CreateObjects()
    vpc.client.CreateObjects()
    policy.client.CreateObjects()
    route.client.CreateObjects()
    subnet.client.CreateObjects()
    vnic.client.CreateObjects()

    # Create Mirror session objects
    #mirror.client.CreateObjects()

    # Create VPC Objects
    #VpcObjectClient(Store.Get)
    #vpc.client.CreateObjects()


    # Commit the Batch
    device.client.Commit()


    return

def Main(args):


    timeprofiler.ConfigTimeProfiler.Start()
    #Have to look into config push
    #agentapi.Init()

    defs.DOL_PATH = "/iota/"
    api.Logger.info("Generating Configuration for Spec  = %s" % args.spec)
    if args.spec == 'dummy':
        return api.types.status.SUCCESS
    cfgpec = parser.ParseFile('test/apollo/config/cfg/', '%s'%args.spec)
    __generate(cfgpec)

    api.Logger.info("Creating objects in Agent")
    __create()
    api.Logger.info("Creating objects in Agent complete")
    timeprofiler.ConfigTimeProfiler.Stop()

    return api.types.status.SUCCESS
