// (c) Pensando Systems, Inc.
// This is a generated file, please do not hand edit !!

package pregen

import (
	api2 "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/venice/cli/api"
)

// GetObjSpec is
func GetObjSpec(objName string) interface{} {
	switch objName {

	case "cluster":
		return cmd.ClusterSpec{}

	case "endpoint":
		return network.EndpointSpec{}

	case "lbPolicy":
		return network.LbPolicySpec{}

	case "network":
		return network.NetworkSpec{}

	case "node":
		return cmd.NodeSpec{}

	case "permission":
		return api.PermissionSpec{}

	case "role":
		return api.RoleSpec{}

	case "securityGroup":
		return network.SecurityGroupSpec{}

	case "service":
		return network.ServiceSpec{}

	case "sgpolicy":
		return network.SgpolicySpec{}

	case "smartNIC":
		return cmd.SmartNICSpec{}

	case "tenant":
		return network.TenantSpec{}

	case "user":
		return api.UserSpec{}

	}
	return nil
}

// GetSubObj is
func GetSubObj(kind string) interface{} {
	switch kind {

	case "Timestamp":
		var v api2.Timestamp
		return &v

	case "HealthCheckSpec":
		var v network.HealthCheckSpec
		return &v

	case "NodeCondition":
		var v cmd.NodeCondition
		return &v

	case "PortCondition":
		var v cmd.PortCondition
		return &v

	case "ConditionStatus":
		var v cmd.ConditionStatus
		return &v

	case "Selector":
		var v labels.Selector
		return &v

	case "Requirement":
		var v labels.Requirement
		return &v

	case "TLSServerPolicySpec":
		var v network.TLSServerPolicySpec
		return &v

	case "TLSClientPolicySpec":
		var v network.TLSClientPolicySpec
		return &v

	case "SGRule":
		var v network.SGRule
		return &v

	case "PortSpec":
		var v cmd.PortSpec
		return &v

	case "PortStatus":
		var v cmd.PortStatus
		return &v

	case "SmartNICCondition":
		var v cmd.SmartNICCondition
		return &v

	case "UserAuditLog":
		var v api.UserAuditLog
		return &v

	}
	return nil
}

// GetObjStatus is
func GetObjStatus(objName string) interface{} {
	switch objName {

	case "cluster":
		o := cmd.Cluster{}
		return o.Status

	case "endpoint":
		o := network.Endpoint{}
		return o.Status

	case "lbPolicy":
		o := network.LbPolicy{}
		return o.Status

	case "network":
		o := network.Network{}
		return o.Status

	case "node":
		o := cmd.Node{}
		return o.Status

	case "permission":
		o := api.Permission{}
		return o.Status

	case "role":
		o := api.Role{}
		return o.Status

	case "securityGroup":
		o := network.SecurityGroup{}
		return o.Status

	case "service":
		o := network.Service{}
		return o.Status

	case "sgpolicy":
		o := network.Sgpolicy{}
		return o.Status

	case "smartNIC":
		o := cmd.SmartNIC{}
		return o.Status

	case "tenant":
		o := network.Tenant{}
		return o.Status

	case "user":
		o := api.User{}
		return o.Status

	}
	return nil
}
