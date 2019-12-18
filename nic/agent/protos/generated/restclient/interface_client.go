// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package netproto is a auto generated package.
Input file: interface.proto
*/
package restclient

import (
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/netutils"
)

// InterfaceList lists all Interface objects
func (cl *AgentClient) InterfaceList() ([]netproto.Interface, error) {
	var interfaceList []netproto.Interface

	err := netutils.HTTPGet("http://"+cl.agentURL+"/api/interfaces/", &interfaceList)

	return interfaceList, err
}
