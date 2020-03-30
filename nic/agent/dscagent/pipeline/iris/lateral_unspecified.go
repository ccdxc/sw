// +build !apulu
// +build !linux

package iris

import (
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
)

//func resolveARPForDefaultGateway(addr net.IP, arpClient *arp.Client) (net.HardwareAddr, error) {
//	return arpClient.Resolve(addr)
//}

// CreateLateralNetAgentObjects is a stubbed out method for non linux systems.
func CreateLateralNetAgentObjects(infraAPI types.InfraAPI, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, vrfID uint64, owner string, mgmtIP, destIP, gwIP string, tunnelOp bool) error {
	return nil
}

// DeleteLateralNetAgentObjects is a stubbed out method for non linux systems.
func DeleteLateralNetAgentObjects(infraAPI types.InfraAPI, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, vrfID uint64, owner string, destIP string, tunnelOp bool) error {
	return nil
}
