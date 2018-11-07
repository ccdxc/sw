// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package networkCliUtilsBackend is a auto generated package.
Input file: service.proto
*/
package cli

import (
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/cli/gen"
)

// CreateServiceFlags specifies flags for Service create operation
var CreateServiceFlags = []gen.CliFlag{
	{
		ID:     "client-authentication",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "lb-policy",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "ports",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "tls-client-allowed-peer-id",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "tls-client-certificates-selector",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "tls-client-trust-roots",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "tls-server-allowed-peer-id",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "tls-server-certificates",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "tls-server-trust-roots",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "virtual-ip",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "workload-labels",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
}

func removeServiceOper(obj interface{}) error {
	if v, ok := obj.(*network.Service); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = network.ServiceStatus{}
	}
	return nil
}

func init() {
	cl := gen.GetInfo()

	cl.AddCliInfo("network.Service", "create", CreateServiceFlags)
	cl.AddRemoveObjOperFunc("network.Service", removeServiceOper)

}
