// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package securityCliUtilsBackend is a auto generated package.
Input file: x509.proto
*/
package cli

import (
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/cli/gen"
)

// CreateCertificateFlags specifies flags for Certificate create operation
var CreateCertificateFlags = []gen.CliFlag{
	{
		ID:     "body",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "description",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "trust-chain",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "usages",
		Type:   "StringSlice",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
}

func removeCertificateOper(obj interface{}) error {
	if v, ok := obj.(*security.Certificate); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = security.CertificateStatus{}
	}
	return nil
}

func init() {
	cl := gen.GetInfo()

	cl.AddCliInfo("security.Certificate", "create", CreateCertificateFlags)
	cl.AddRemoveObjOperFunc("security.Certificate", removeCertificateOper)

}
