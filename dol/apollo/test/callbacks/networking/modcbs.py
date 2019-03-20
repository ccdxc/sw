#! /usr/bin/python3
from infra.common.logging import logger as logger

def Setup(infra, module):
    iterelem = module.iterator.Get()

    logger.info("Iterator Selectors")
    if iterelem:
        if 'flow' in iterelem.__dict__:
            logger.info("Extending flow selector to : %s" % iterelem.flow)
            module.testspec.selectors.flow.Extend(iterelem.flow)
