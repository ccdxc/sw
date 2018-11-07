// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package securityCliUtilsBackend is a auto generated package.
Input file: securitygroup.proto
*/
package cli

import (
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/cli/gen"
)

// CreateSecurityGroupFlags specifies flags for SecurityGroup create operation
var CreateSecurityGroupFlags = []gen.CliFlag{
	{
		ID:     "match-prefixes",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "service-labels",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
}

func removeSecurityGroupOper(obj interface{}) error {
	if v, ok := obj.(*security.SecurityGroup); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = security.SecurityGroupStatus{}
	}
	return nil
}

func init() {
	cl := gen.GetInfo()

	cl.AddCliInfo("security.SecurityGroup", "create", CreateSecurityGroupFlags)
	cl.AddRemoveObjOperFunc("security.SecurityGroup", removeSecurityGroupOper)

}
