// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package diagnosticsCliUtilsBackend is a auto generated package.
Input file: module.proto
*/
package cli

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/cli/gen"
)

// CreateModuleFlags specifies flags for Module create operation
var CreateModuleFlags = []gen.CliFlag{
	{
		ID:     "args",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "enable-trace",
		Type:   "Bool",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "log-level",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
}

func removeModuleOper(obj interface{}) error {
	if v, ok := obj.(*diagnostics.Module); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.CreationTime = api.Timestamp{}
		v.ModTime = api.Timestamp{}
		v.Status = diagnostics.ModuleStatus{}
	}
	return nil
}

func init() {
	cl := gen.GetInfo()

	cl.AddCliInfo("diagnostics.Module", "create", CreateModuleFlags)
	cl.AddRemoveObjOperFunc("diagnostics.Module", removeModuleOper)

}
