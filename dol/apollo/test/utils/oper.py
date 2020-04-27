#! /usr/bin/python3
import pdb

from infra.common.logging import logger
import apollo.config.utils as utils

def GetNumVnic(node, subnet=None):
    if subnet:
        args = f" | grep {subnet.UUID.UuidStr}"
    else:
        args = None
    ret, cli_op = utils.RunPdsctlShowCmd(node, "vnic", args, yaml=False)
    if not ret:
        logger.error(f"show vnic failed for node {node}")
        return 0
    return len(cli_op.split("\n"))
    
def GetNumLearnMac(node, subnet=None):
    if subnet:
        args = f" | grep {subnet.UUID.UuidStr}"
    else:
        args = None
    ret, cli_op = utils.RunPdsctlShowCmd(node, "learn mac", args, yaml=False)
    if not ret:
        logger.error(f"show learn mac failed for node {node}")
        return 0
    return len(cli_op.split("\n"))

def GetNumLearnIp(node, subnet=None):
    if subnet:
        args = f"--subnet {subnet.UUID.UuidStr}"
    else:
        args = ""
    ret, cli_op = utils.RunPdsctlShowCmd(node, "learn ip", args, yaml=True)
    if not ret:
        logger.error(f"show learn ip failed for node {node}")
        return -1
    return (len(cli_op.split("---")) - 1)

def GetNumLocalMapping(node):
    ret, cli_op = utils.RunPdsctlShowCmd(node, "mapping internal local", yaml=False)
    if not ret:
        logger.error(f"show mapping internal local failed for node {node}")
        return -1
    return (len(cli_op.split('\n')) - 4) # skipping the header lines in the output
