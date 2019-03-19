#! /usr/bin/python3
from infra.common.logging import logger as logger

def Setup(infra, module):
    iterelem = module.iterator.Get()

    logger.info("Iterator Selectors")
    if iterelem:
        if 'mapping' in iterelem.__dict__:
            logger.info("- mapping: %s" % iterelem.mapping)
            module.testspec.selectors.mapping.Extend(iterelem.mapping)
