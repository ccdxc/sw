// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package clusterCliUtilsBackend is a auto generated package.
Input file: tenant.proto
*/
package cli

import (
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cli/gen"
)

// CreateTenantFlags specifies flags for Tenant create operation
var CreateTenantFlags = []gen.CliFlag{
	{
		ID:     "admin-user",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
}

func removeTenantOper(obj interface{}) error {
	if v, ok := obj.(*cluster.Tenant); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = cluster.TenantStatus{}
	}
	return nil
}

func init() {
	cl := gen.GetInfo()

	cl.AddCliInfo("cluster.Tenant", "create", CreateTenantFlags)
	cl.AddRemoveObjOperFunc("cluster.Tenant", removeTenantOper)

}
