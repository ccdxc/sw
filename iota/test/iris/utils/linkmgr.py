#! /usr/bin/python3
import iota.harness.api as api
import yaml
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

def GetLinkStatusChangeCount (naples_host):
    #there should be a change to that function that is truck through ttps://pensando.atlassian.net/browse/PS-504
    #for now - lets just scrab through the log
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "/nic/bin/halctl  show port  --yaml"
    api.Trigger_AddNaplesCommand(req, naples_host, cmd)
    resp = api.Trigger(req)
    if resp == None:
       	api.Logger.error("Bad response from Naples, while trying to read the output" )
        return None
    command = resp.commands[0]
#    api.PrintCommandResults(command);
    perPortOutput = command.stdout.split("---")
    totalLinkFlaps = 0
    successfulYamlParsing = 0
    for portInfo in perPortOutput:
        try:
            testobj = yaml.load(portInfo, Loader=yaml.Loader)#parse the YAML
            if bool(testobj):#last split is always empty - skip it. Otherwise process it
                 portId=testobj['spec']['keyorhandle']['keyorhandle']['portid']
                 linkDownPerPort=testobj['stats']['numlinkdown']
                 api.Logger.info("Node Name:[%s] Port:[%d] Number of total Link flaps:[%d] "% (naples_host,portId,linkDownPerPort))
                 totalLinkFlaps +=linkDownPerPort
                 successfulYamlParsing=1
        except :
            api.Logger.info("The following entry is not in YAML format" +portInfo)
    if successfulYamlParsing==1:
        api.Logger.info("Total number of link flaps detected is " +str(totalLinkFlaps))
    else:   
        if GlobalOptions.dryrun: return 0
        api.Logger.error("Un expected error while paring YAML output of port status Unable to find any entry that indicates number of link flapped. May be your version of firmware is outdated or the output of the function has changed" )
        api.PrintCommandResults(command);
        totalLinkFlaps=None
    if GlobalOptions.dryrun: return 0
    return totalLinkFlaps

def ResetLastLinkStatusChangeCount(naples_host,newValue):
    api.SetTestsuiteAttr("LinkTestValidationNumberOfFlaps"+naples_host,newValue) 

def GetLastLinkStatusChangeCount(naples_host):
    return api.GetTestsuiteAttr("LinkTestValidationNumberOfFlaps"+naples_host)

