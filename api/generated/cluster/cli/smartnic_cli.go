// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package clusterCliUtilsBackend is a auto generated package.
Input file: smartnic.proto
*/
package cli

import (
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cli/gen"
)

// CreateSmartNICFlags specifies flags for SmartNIC create operation
var CreateSmartNICFlags = []gen.CliFlag{
	{
		ID:     "admit",
		Type:   "Bool",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "controllers",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "hostname",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "mgmt-mode",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "mgmt-vlan",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
}

func removeSmartNICOper(obj interface{}) error {
	if v, ok := obj.(*cluster.SmartNIC); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = cluster.SmartNICStatus{}
	}
	return nil
}

func init() {
	cl := gen.GetInfo()

	cl.AddCliInfo("cluster.SmartNIC", "create", CreateSmartNICFlags)
	cl.AddRemoveObjOperFunc("cluster.SmartNIC", removeSmartNICOper)

}
