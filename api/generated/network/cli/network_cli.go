// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package networkCliUtilsBackend is a auto generated package.
Input file: network.proto
*/
package cli

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/cli/gen"
)

// CreateNetworkFlags specifies flags for Network create operation
var CreateNetworkFlags = []gen.CliFlag{
	{
		ID:     "egress-security-policy",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "ingress-security-policy",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "ipam-policy",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "ipv4-gateway",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "ipv4-subnet",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "ipv6-gateway",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "ipv6-subnet",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "namespace",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "orchestrator-name",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "type",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "virtual-router",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "vlan-id",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "vxlan-vni",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
}

func removeNetworkOper(obj interface{}) error {
	if v, ok := obj.(*network.Network); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.CreationTime = api.Timestamp{}
		v.ModTime = api.Timestamp{}
		v.Status = network.NetworkStatus{}
	}
	return nil
}

func init() {
	cl := gen.GetInfo()

	cl.AddCliInfo("network.Network", "create", CreateNetworkFlags)
	cl.AddRemoveObjOperFunc("network.Network", removeNetworkOper)

}
