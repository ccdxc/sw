#! /usr/bin/python3
import iota.harness.api as api
def Main(step):
    topo = api.ParseYml(step.topology)
    api.logger.info("Bringing up topology: %s" % topo)
    
    api.InitTestbed(topo.switch_port_id,
                    topo.naples_image,
                    topo.venice_image,
                    topo.driver_sources,
                    topo.iota_agent_image)

    for node in topo.nodes:
        
    return
