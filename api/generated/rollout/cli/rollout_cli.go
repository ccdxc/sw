// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package rolloutCliUtilsBackend is a auto generated package.
Input file: rollout.proto
*/
package cli

import (
	"github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/venice/cli/gen"
)

// CreateRolloutFlags specifies flags for Rollout create operation
var CreateRolloutFlags = []gen.CliFlag{
	{
		ID:     "duration",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "max-nic-failures-before-abort",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "max-parallel",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "smartnic-must-match-constraint",
		Type:   "Bool",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "smartnics-only",
		Type:   "Bool",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "strategy",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "suspend",
		Type:   "Bool",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "upgrade-type",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
	{
		ID:     "version",
		Type:   "String",
		Help:   "",
		Skip:   false,
		Insert: "",
	},
}

func removeRolloutOper(obj interface{}) error {
	if v, ok := obj.(*rollout.Rollout); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = rollout.RolloutStatus{}
	}
	return nil
}

func init() {
	cl := gen.GetInfo()

	cl.AddCliInfo("rollout.Rollout", "create", CreateRolloutFlags)
	cl.AddRemoveObjOperFunc("rollout.Rollout", removeRolloutOper)

}
