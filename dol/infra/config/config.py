#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.common.dyml        as dyml
import infra.common.utils       as utils
import infra.common.parser      as parser

from infra.common.logging       import logger
from infra.config.parser        import ConfigParser as ConfigParser

def Init(store, template_path, spec_path, topo_spec_path):
    logger.info("Loading Config Object Templates.")
    objlist = ConfigParser.ParseTemplates(store, template_path)
    store.templates.SetAll(objlist)

    logger.info("Loading CONFIG Object Specs")
    specs = ConfigParser.ParseSpecs(store, spec_path, topo_spec_path)
    store.specs.SetAll(specs)
    return
