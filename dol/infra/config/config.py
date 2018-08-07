#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.common.dyml        as dyml
import infra.common.utils       as utils
import infra.common.parser      as parser

from infra.common.logging       import logger
from infra.config.parser        import ConfigParser as ConfigParser
from iris.config.store          import Store

def init():
    logger.info("Loading Config Object Templates.")
    objlist = ConfigParser.ParseTemplates(Store)
    Store.templates.SetAll(objlist)

    logger.info("Loading CONFIG Object Specs")
    specs = ConfigParser.ParseSpecs(Store)
    Store.specs.SetAll(specs)
    return


