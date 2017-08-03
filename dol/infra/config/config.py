#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.common.dyml        as dyml
import infra.common.utils       as utils
import infra.common.parser      as parser
import config.hal.api            as halapi

from infra.common.logging       import cfglogger
from infra.config.parser        import ConfigParser as ConfigParser
from config.store               import Store

import config.objects.uplink    as uplink
import config.objects.uplinkpc  as uplinkpc
import config.objects.tenant    as tenant
import config.objects.segment   as segment
import config.objects.endpoint  as endpoint
import config.objects.enic      as enic
import config.objects.session   as session
import config.objects.security_profile as security_profile

def init():
    cfglogger.info("Loading Config Object Templates.")
    objlist = ConfigParser.ParseTemplates(Store)
    Store.templates.SetAll(objlist)

    cfglogger.info("Loading CONFIG Object Specs")
    specs = ConfigParser.ParseSpecs(Store)
    Store.specs.SetAll(specs)
    return


