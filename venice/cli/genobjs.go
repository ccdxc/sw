// (c) Pensando Systems, Inc.
// This is a generated file, please do not hand edit !!

package vcli

import (
	contxt "context"
	"encoding/json"
	"fmt"
	"strings"

	log "github.com/sirupsen/logrus"

	swapi "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/api/generated/network"

	api2 "github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cli/api"
	"github.com/pensando/sw/venice/cli/gen/pregen"
	"github.com/pensando/sw/venice/utils/ref"
)

func getObj(ctx *context) (obj interface{}, objList interface{}) {
	switch ctx.subcmd {

	case "cluster":
		return &cmd.Cluster{}, &cmd.ClusterList{}

	case "endpoint":
		return &network.Endpoint{}, &network.EndpointList{}

	case "lbPolicy":
		return &network.LbPolicy{}, &network.LbPolicyList{}

	case "network":
		return &network.Network{}, &network.NetworkList{}

	case "node":
		return &cmd.Node{}, &cmd.NodeList{}

	case "permission":
		return &api.Permission{}, &api.PermissionList{}

	case "role":
		return &api.Role{}, &api.RoleList{}

	case "securityGroup":
		return &network.SecurityGroup{}, &network.SecurityGroupList{}

	case "service":
		return &network.Service{}, &network.ServiceList{}

	case "sgpolicy":
		return &network.Sgpolicy{}, &network.SgpolicyList{}

	case "smartNIC":
		return &cmd.SmartNIC{}, &cmd.SmartNICList{}

	case "tenant":
		return &network.Tenant{}, &network.TenantList{}

	case "user":
		return &api.User{}, &api.UserList{}

	}
	return nil, nil
}

func getSubObj(kind string) interface{} {
	switch kind {

	case "Timestamp":
		var v api2.Timestamp
		return &v

	case "HealthCheckSpec":
		var v network.HealthCheckSpec
		return v

	case "NodeCondition":
		var v cmd.NodeCondition
		return v

	case "PortCondition":
		var v cmd.PortCondition
		return v

	case "ConditionStatus":
		var v cmd.ConditionStatus
		return v

	case "SGRule":
		var v network.SGRule
		return v

	case "PortSpec":
		var v cmd.PortSpec
		return v

	case "PortStatus":
		var v cmd.PortStatus
		return v

	case "SmartNICCondition":
		var v cmd.SmartNICCondition
		return v

	case "UserAuditLog":
		var v api.UserAuditLog
		return v

	}
	return nil
}

func getObjFromList(objList interface{}, idx int) interface{} {

	if ol, ok := objList.(*cmd.ClusterList); ok {
		cluster := ol.Items[idx]
		return cluster
	}

	if ol, ok := objList.(*network.EndpointList); ok {
		endpoint := ol.Items[idx]
		return endpoint
	}

	if ol, ok := objList.(*network.LbPolicyList); ok {
		lbPolicy := ol.Items[idx]
		return lbPolicy
	}

	if ol, ok := objList.(*network.NetworkList); ok {
		network := ol.Items[idx]
		return network
	}

	if ol, ok := objList.(*cmd.NodeList); ok {
		node := ol.Items[idx]
		return node
	}

	if ol, ok := objList.(*api.PermissionList); ok {
		permission := ol.Items[idx]
		return permission
	}

	if ol, ok := objList.(*api.RoleList); ok {
		role := ol.Items[idx]
		return role
	}

	if ol, ok := objList.(*network.SecurityGroupList); ok {
		securityGroup := ol.Items[idx]
		return securityGroup
	}

	if ol, ok := objList.(*network.ServiceList); ok {
		service := ol.Items[idx]
		return service
	}

	if ol, ok := objList.(*network.SgpolicyList); ok {
		sgpolicy := ol.Items[idx]
		return sgpolicy
	}

	if ol, ok := objList.(*cmd.SmartNICList); ok {
		smartNIC := ol.Items[idx]
		return smartNIC
	}

	if ol, ok := objList.(*network.TenantList); ok {
		tenant := ol.Items[idx]
		return tenant
	}

	if ol, ok := objList.(*api.UserList); ok {
		user := ol.Items[idx]
		return user
	}

	return nil
}

func removeObjOper(obj interface{}) error {

	if v, ok := obj.(*cmd.Cluster); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = cmd.ClusterStatus{}
	}

	if v, ok := obj.(*network.Endpoint); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = network.EndpointStatus{}
	}

	if v, ok := obj.(*network.LbPolicy); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = network.LbPolicyStatus{}
	}

	if v, ok := obj.(*network.Network); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = network.NetworkStatus{}
	}

	if v, ok := obj.(*cmd.Node); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = cmd.NodeStatus{}
	}

	if v, ok := obj.(*api.Permission); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = api.PermissionStatus{}
	}

	if v, ok := obj.(*api.Role); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = api.RoleStatus{}
	}

	if v, ok := obj.(*network.SecurityGroup); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = network.SecurityGroupStatus{}
	}

	if v, ok := obj.(*network.Service); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = network.ServiceStatus{}
	}

	if v, ok := obj.(*network.Sgpolicy); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = network.SgpolicyStatus{}
	}

	if v, ok := obj.(*cmd.SmartNIC); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = cmd.SmartNICStatus{}
	}

	if v, ok := obj.(*network.Tenant); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = network.TenantStatus{}
	}

	if v, ok := obj.(*api.User); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = api.UserStatus{}
	}

	return nil
}

func writeObj(obj interface{}, objmKvs, specKvs map[string]ref.FInfo) interface{} {

	if v, ok := obj.(*cmd.Cluster); ok {
		return writeClusterObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*network.Endpoint); ok {
		return writeEndpointObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*network.LbPolicy); ok {
		return writeLbPolicyObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*network.Network); ok {
		return writeNetworkObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*cmd.Node); ok {
		return writeNodeObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*api.Permission); ok {
		return writePermissionObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*api.Role); ok {
		return writeRoleObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*network.SecurityGroup); ok {
		return writeSecurityGroupObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*network.Service); ok {
		return writeServiceObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*network.Sgpolicy); ok {
		return writeSgpolicyObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*cmd.SmartNIC); ok {
		return writeSmartNICObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*network.Tenant); ok {
		return writeTenantObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*api.User); ok {
		return writeUserObj(*v, objmKvs, specKvs)
	}

	return nil
}

func getAllKvs(ctx *context, objList interface{}) ([]map[string]ref.FInfo, []map[string]ref.FInfo, map[string]bool, map[string]bool) {
	objmKvs := []map[string]ref.FInfo{}
	specKvs := []map[string]ref.FInfo{}
	objmValidKvs := make(map[string]bool)
	specValidKvs := make(map[string]bool)
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	switch ctx.subcmd {

	case "cluster":
		clusterList := objList.(*cmd.ClusterList)
		for idx, o := range clusterList.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}

	case "endpoint":
		endpointList := objList.(*network.EndpointList)
		for idx, o := range endpointList.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}

	case "lbPolicy":
		lbPolicyList := objList.(*network.LbPolicyList)
		for idx, o := range lbPolicyList.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}

	case "network":
		networkList := objList.(*network.NetworkList)
		for idx, o := range networkList.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}

	case "node":
		nodeList := objList.(*cmd.NodeList)
		for idx, o := range nodeList.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}

	case "permission":
		permissionList := objList.(*api.PermissionList)
		for idx, o := range permissionList.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}

	case "role":
		roleList := objList.(*api.RoleList)
		for idx, o := range roleList.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}

	case "securityGroup":
		securityGroupList := objList.(*network.SecurityGroupList)
		for idx, o := range securityGroupList.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}

	case "service":
		serviceList := objList.(*network.ServiceList)
		for idx, o := range serviceList.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}

	case "sgpolicy":
		sgpolicyList := objList.(*network.SgpolicyList)
		for idx, o := range sgpolicyList.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}

	case "smartNIC":
		smartNICList := objList.(*cmd.SmartNICList)
		for idx, o := range smartNICList.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}

	case "tenant":
		tenantList := objList.(*network.TenantList)
		for idx, o := range tenantList.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}

	case "user":
		userList := objList.(*api.UserList)
		for idx, o := range userList.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}

	}

	return objmKvs, specKvs, objmValidKvs, specValidKvs
}

func writeTimestampObj(obj api2.Timestamp, specKvs map[string]ref.FInfo) *api2.Timestamp {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newTimestamp := new.(api2.Timestamp)

	return &newTimestamp
}

func writeHealthCheckSpecObj(obj network.HealthCheckSpec, specKvs map[string]ref.FInfo) *network.HealthCheckSpec {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newHealthCheckSpec := new.(network.HealthCheckSpec)

	return &newHealthCheckSpec
}

func writeNodeConditionObj(obj cmd.NodeCondition, specKvs map[string]ref.FInfo) *cmd.NodeCondition {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newNodeCondition := new.(cmd.NodeCondition)

	return &newNodeCondition
}

func writePortConditionObj(obj cmd.PortCondition, specKvs map[string]ref.FInfo) *cmd.PortCondition {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newPortCondition := new.(cmd.PortCondition)

	return &newPortCondition
}

func writeConditionStatusObj(obj cmd.ConditionStatus, specKvs map[string]ref.FInfo) *cmd.ConditionStatus {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newConditionStatus := new.(cmd.ConditionStatus)

	return &newConditionStatus
}

func writeSGRuleObj(obj network.SGRule, specKvs map[string]ref.FInfo) *network.SGRule {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newSGRule := new.(network.SGRule)

	return &newSGRule
}

func writePortSpecObj(obj cmd.PortSpec, specKvs map[string]ref.FInfo) *cmd.PortSpec {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newPortSpec := new.(cmd.PortSpec)

	return &newPortSpec
}

func writePortStatusObj(obj cmd.PortStatus, specKvs map[string]ref.FInfo) *cmd.PortStatus {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newPortStatus := new.(cmd.PortStatus)

	return &newPortStatus
}

func writeSmartNICConditionObj(obj cmd.SmartNICCondition, specKvs map[string]ref.FInfo) *cmd.SmartNICCondition {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newSmartNICCondition := new.(cmd.SmartNICCondition)

	return &newSmartNICCondition
}

func writeUserAuditLogObj(obj api.UserAuditLog, specKvs map[string]ref.FInfo) *api.UserAuditLog {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newUserAuditLog := new.(api.UserAuditLog)

	return &newUserAuditLog
}

func writeClusterObj(obj cmd.Cluster, metaKvs, specKvs map[string]ref.FInfo) *cmd.Cluster {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newCluster := &cmd.Cluster{
		TypeMeta:   swapi.TypeMeta{Kind: "cluster"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(cmd.ClusterSpec),
	}
	return newCluster
}

func writeEndpointObj(obj network.Endpoint, metaKvs, specKvs map[string]ref.FInfo) *network.Endpoint {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newEndpoint := &network.Endpoint{
		TypeMeta:   swapi.TypeMeta{Kind: "endpoint"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(network.EndpointSpec),
	}
	return newEndpoint
}

func writeLbPolicyObj(obj network.LbPolicy, metaKvs, specKvs map[string]ref.FInfo) *network.LbPolicy {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newLbPolicy := &network.LbPolicy{
		TypeMeta:   swapi.TypeMeta{Kind: "lbPolicy"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(network.LbPolicySpec),
	}
	return newLbPolicy
}

func writeNetworkObj(obj network.Network, metaKvs, specKvs map[string]ref.FInfo) *network.Network {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newNetwork := &network.Network{
		TypeMeta:   swapi.TypeMeta{Kind: "network"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(network.NetworkSpec),
	}
	return newNetwork
}

func writeNodeObj(obj cmd.Node, metaKvs, specKvs map[string]ref.FInfo) *cmd.Node {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newNode := &cmd.Node{
		TypeMeta:   swapi.TypeMeta{Kind: "node"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(cmd.NodeSpec),
	}
	return newNode
}

func writePermissionObj(obj api.Permission, metaKvs, specKvs map[string]ref.FInfo) *api.Permission {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newPermission := &api.Permission{
		TypeMeta:   swapi.TypeMeta{Kind: "permission"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(api.PermissionSpec),
	}
	return newPermission
}

func writeRoleObj(obj api.Role, metaKvs, specKvs map[string]ref.FInfo) *api.Role {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newRole := &api.Role{
		TypeMeta:   swapi.TypeMeta{Kind: "role"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(api.RoleSpec),
	}
	return newRole
}

func writeSecurityGroupObj(obj network.SecurityGroup, metaKvs, specKvs map[string]ref.FInfo) *network.SecurityGroup {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newSecurityGroup := &network.SecurityGroup{
		TypeMeta:   swapi.TypeMeta{Kind: "securityGroup"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(network.SecurityGroupSpec),
	}
	return newSecurityGroup
}

func writeServiceObj(obj network.Service, metaKvs, specKvs map[string]ref.FInfo) *network.Service {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newService := &network.Service{
		TypeMeta:   swapi.TypeMeta{Kind: "service"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(network.ServiceSpec),
	}
	return newService
}

func writeSgpolicyObj(obj network.Sgpolicy, metaKvs, specKvs map[string]ref.FInfo) *network.Sgpolicy {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newSgpolicy := &network.Sgpolicy{
		TypeMeta:   swapi.TypeMeta{Kind: "sgpolicy"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(network.SgpolicySpec),
	}
	return newSgpolicy
}

func writeSmartNICObj(obj cmd.SmartNIC, metaKvs, specKvs map[string]ref.FInfo) *cmd.SmartNIC {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newSmartNIC := &cmd.SmartNIC{
		TypeMeta:   swapi.TypeMeta{Kind: "smartNIC"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(cmd.SmartNICSpec),
	}
	return newSmartNIC
}

func writeTenantObj(obj network.Tenant, metaKvs, specKvs map[string]ref.FInfo) *network.Tenant {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newTenant := &network.Tenant{
		TypeMeta:   swapi.TypeMeta{Kind: "tenant"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(network.TenantSpec),
	}
	return newTenant
}

func writeUserObj(obj api.User, metaKvs, specKvs map[string]ref.FInfo) *api.User {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newUser := &api.User{
		TypeMeta:   swapi.TypeMeta{Kind: "user"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(api.UserSpec),
	}
	return newUser
}

func createObjFromBytes(ctx *context, objName, inp string) error {
	switch ctx.subcmd {

	case "cluster":
		createClusterFromBytes(ctx, inp)

	case "endpoint":
		createEndpointFromBytes(ctx, inp)

	case "lbPolicy":
		createLbPolicyFromBytes(ctx, inp)

	case "network":
		createNetworkFromBytes(ctx, inp)

	case "node":
		createNodeFromBytes(ctx, inp)

	case "permission":
		createPermissionFromBytes(ctx, inp)

	case "role":
		createRoleFromBytes(ctx, inp)

	case "securityGroup":
		createSecurityGroupFromBytes(ctx, inp)

	case "service":
		createServiceFromBytes(ctx, inp)

	case "sgpolicy":
		createSgpolicyFromBytes(ctx, inp)

	case "smartNIC":
		createSmartNICFromBytes(ctx, inp)

	case "tenant":
		createTenantFromBytes(ctx, inp)

	case "user":
		createUserFromBytes(ctx, inp)

	}
	return nil
}

func createClusterFromBytes(ctx *context, inp string) error {
	cluster := &cmd.Cluster{}
	if err := json.Unmarshal([]byte(inp), cluster); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	cluster.Tenant = ctx.tenant
	if err := postObj(ctx, cluster, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createEndpointFromBytes(ctx *context, inp string) error {
	endpoint := &network.Endpoint{}
	if err := json.Unmarshal([]byte(inp), endpoint); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	endpoint.Tenant = ctx.tenant
	if err := postObj(ctx, endpoint, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createLbPolicyFromBytes(ctx *context, inp string) error {
	lbPolicy := &network.LbPolicy{}
	if err := json.Unmarshal([]byte(inp), lbPolicy); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	lbPolicy.Tenant = ctx.tenant
	if err := postObj(ctx, lbPolicy, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createNetworkFromBytes(ctx *context, inp string) error {
	network := &network.Network{}
	if err := json.Unmarshal([]byte(inp), network); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	network.Tenant = ctx.tenant
	if err := postObj(ctx, network, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createNodeFromBytes(ctx *context, inp string) error {
	node := &cmd.Node{}
	if err := json.Unmarshal([]byte(inp), node); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	node.Tenant = ctx.tenant
	if err := postObj(ctx, node, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createPermissionFromBytes(ctx *context, inp string) error {
	permission := &api.Permission{}
	if err := json.Unmarshal([]byte(inp), permission); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	permission.Tenant = ctx.tenant
	if err := postObj(ctx, permission, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createRoleFromBytes(ctx *context, inp string) error {
	role := &api.Role{}
	if err := json.Unmarshal([]byte(inp), role); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	role.Tenant = ctx.tenant
	if err := postObj(ctx, role, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createSecurityGroupFromBytes(ctx *context, inp string) error {
	securityGroup := &network.SecurityGroup{}
	if err := json.Unmarshal([]byte(inp), securityGroup); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	securityGroup.Tenant = ctx.tenant
	if err := postObj(ctx, securityGroup, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createServiceFromBytes(ctx *context, inp string) error {
	service := &network.Service{}
	if err := json.Unmarshal([]byte(inp), service); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	service.Tenant = ctx.tenant
	if err := postObj(ctx, service, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createSgpolicyFromBytes(ctx *context, inp string) error {
	sgpolicy := &network.Sgpolicy{}
	if err := json.Unmarshal([]byte(inp), sgpolicy); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	sgpolicy.Tenant = ctx.tenant
	if err := postObj(ctx, sgpolicy, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createSmartNICFromBytes(ctx *context, inp string) error {
	smartNIC := &cmd.SmartNIC{}
	if err := json.Unmarshal([]byte(inp), smartNIC); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	smartNIC.Tenant = ctx.tenant
	if err := postObj(ctx, smartNIC, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createTenantFromBytes(ctx *context, inp string) error {
	tenant := &network.Tenant{}
	if err := json.Unmarshal([]byte(inp), tenant); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	tenant.Tenant = ctx.tenant
	if err := postObj(ctx, tenant, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createUserFromBytes(ctx *context, inp string) error {
	user := &api.User{}
	if err := json.Unmarshal([]byte(inp), user); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	user.Tenant = ctx.tenant
	if err := postObj(ctx, user, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func updateLabel(obj interface{}, newLabels map[string]string) error {

	if o, ok := obj.(*cmd.Cluster); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*network.Endpoint); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*network.LbPolicy); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*network.Network); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*cmd.Node); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*api.Permission); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*api.Role); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*network.SecurityGroup); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*network.Service); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*network.Sgpolicy); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*cmd.SmartNIC); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*network.Tenant); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*api.User); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	return fmt.Errorf("unknown object")
}

func restGet(url, tenant string, obj interface{}) error {
	log.Debugf("get url: %s", url)

	urlStrs := strings.Split(url, "/")
	if len(urlStrs) < 3 {
		return fmt.Errorf("invalid url: '%s'", url)
	}
	objName := urlStrs[len(urlStrs)-1]
	hostName := strings.Join(urlStrs[:3], "/")

	restcl, err := apiclient.NewRestAPIClient(hostName)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	ctx := contxt.Background()

	if v, ok := obj.(*cmd.Cluster); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.CmdV1().Cluster().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*cmd.ClusterList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.CmdV1().Cluster().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*network.Endpoint); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.EndpointV1().Endpoint().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*network.EndpointList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.EndpointV1().Endpoint().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*network.LbPolicy); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.LbPolicyV1().LbPolicy().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*network.LbPolicyList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.LbPolicyV1().LbPolicy().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*network.Network); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.NetworkV1().Network().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*network.NetworkList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.NetworkV1().Network().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*cmd.Node); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.CmdV1().Node().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*cmd.NodeList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.CmdV1().Node().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*network.SecurityGroup); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.SecurityGroupV1().SecurityGroup().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*network.SecurityGroupList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.SecurityGroupV1().SecurityGroup().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*network.Service); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.ServiceV1().Service().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*network.ServiceList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.ServiceV1().Service().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*network.Sgpolicy); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.SgpolicyV1().Sgpolicy().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*network.SgpolicyList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.SgpolicyV1().Sgpolicy().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*cmd.SmartNIC); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.CmdV1().SmartNIC().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*cmd.SmartNICList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.CmdV1().SmartNIC().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*network.Tenant); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.TenantV1().Tenant().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*network.TenantList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.TenantV1().Tenant().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	return httpGet(url, obj)
}

func restDelete(objKind, url, tenant string) error {
	log.Debugf("delete url: %s", url)

	urlStrs := strings.Split(url, "/")
	if len(urlStrs) < 3 {
		return fmt.Errorf("invalid url: '%s'", url)
	}
	objName := urlStrs[len(urlStrs)-1]
	hostName := strings.Join(urlStrs[:3], "/")

	restcl, err := apiclient.NewRestAPIClient(hostName)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	ctx := contxt.Background()

	if objKind == "cluster" {
		objm := swapi.ObjectMeta{}
		objm.Name = objName
		objm.Tenant = tenant
		obj, err := restcl.CmdV1().Cluster().Delete(ctx, &objm)
		if err != nil {
			return err
		}
		out, err := json.Marshal(obj)
		if err == nil {
			fmt.Printf("%s", string(out))
		} else {
			fmt.Printf("Unable to marshal object %+v\n", obj)
		}
		return nil
	}

	if objKind == "endpoint" {
		objm := swapi.ObjectMeta{}
		objm.Name = objName
		objm.Tenant = tenant
		obj, err := restcl.EndpointV1().Endpoint().Delete(ctx, &objm)
		if err != nil {
			return err
		}
		out, err := json.Marshal(obj)
		if err == nil {
			fmt.Printf("%s", string(out))
		} else {
			fmt.Printf("Unable to marshal object %+v\n", obj)
		}
		return nil
	}

	if objKind == "lbPolicy" {
		objm := swapi.ObjectMeta{}
		objm.Name = objName
		objm.Tenant = tenant
		obj, err := restcl.LbPolicyV1().LbPolicy().Delete(ctx, &objm)
		if err != nil {
			return err
		}
		out, err := json.Marshal(obj)
		if err == nil {
			fmt.Printf("%s", string(out))
		} else {
			fmt.Printf("Unable to marshal object %+v\n", obj)
		}
		return nil
	}

	if objKind == "network" {
		objm := swapi.ObjectMeta{}
		objm.Name = objName
		objm.Tenant = tenant
		obj, err := restcl.NetworkV1().Network().Delete(ctx, &objm)
		if err != nil {
			return err
		}
		out, err := json.Marshal(obj)
		if err == nil {
			fmt.Printf("%s", string(out))
		} else {
			fmt.Printf("Unable to marshal object %+v\n", obj)
		}
		return nil
	}

	if objKind == "node" {
		objm := swapi.ObjectMeta{}
		objm.Name = objName
		objm.Tenant = tenant
		obj, err := restcl.CmdV1().Node().Delete(ctx, &objm)
		if err != nil {
			return err
		}
		out, err := json.Marshal(obj)
		if err == nil {
			fmt.Printf("%s", string(out))
		} else {
			fmt.Printf("Unable to marshal object %+v\n", obj)
		}
		return nil
	}

	if objKind == "securityGroup" {
		objm := swapi.ObjectMeta{}
		objm.Name = objName
		objm.Tenant = tenant
		obj, err := restcl.SecurityGroupV1().SecurityGroup().Delete(ctx, &objm)
		if err != nil {
			return err
		}
		out, err := json.Marshal(obj)
		if err == nil {
			fmt.Printf("%s", string(out))
		} else {
			fmt.Printf("Unable to marshal object %+v\n", obj)
		}
		return nil
	}

	if objKind == "service" {
		objm := swapi.ObjectMeta{}
		objm.Name = objName
		objm.Tenant = tenant
		obj, err := restcl.ServiceV1().Service().Delete(ctx, &objm)
		if err != nil {
			return err
		}
		out, err := json.Marshal(obj)
		if err == nil {
			fmt.Printf("%s", string(out))
		} else {
			fmt.Printf("Unable to marshal object %+v\n", obj)
		}
		return nil
	}

	if objKind == "sgpolicy" {
		objm := swapi.ObjectMeta{}
		objm.Name = objName
		objm.Tenant = tenant
		obj, err := restcl.SgpolicyV1().Sgpolicy().Delete(ctx, &objm)
		if err != nil {
			return err
		}
		out, err := json.Marshal(obj)
		if err == nil {
			fmt.Printf("%s", string(out))
		} else {
			fmt.Printf("Unable to marshal object %+v\n", obj)
		}
		return nil
	}

	if objKind == "smartNIC" {
		objm := swapi.ObjectMeta{}
		objm.Name = objName
		objm.Tenant = tenant
		obj, err := restcl.CmdV1().SmartNIC().Delete(ctx, &objm)
		if err != nil {
			return err
		}
		out, err := json.Marshal(obj)
		if err == nil {
			fmt.Printf("%s", string(out))
		} else {
			fmt.Printf("Unable to marshal object %+v\n", obj)
		}
		return nil
	}

	if objKind == "tenant" {
		objm := swapi.ObjectMeta{}
		objm.Name = objName
		objm.Tenant = tenant
		obj, err := restcl.TenantV1().Tenant().Delete(ctx, &objm)
		if err != nil {
			return err
		}
		out, err := json.Marshal(obj)
		if err == nil {
			fmt.Printf("%s", string(out))
		} else {
			fmt.Printf("Unable to marshal object %+v\n", obj)
		}
		return nil
	}

	return httpDelete(url)
}

func restPost(url, tenant string, obj interface{}) error {
	log.Debugf("post url: %s", url)

	urlStrs := strings.Split(url, "/")
	if len(urlStrs) < 3 {
		return fmt.Errorf("invalid url: '%s'", url)
	}
	hostName := strings.Join(urlStrs[:3], "/")

	restcl, err := apiclient.NewRestAPIClient(hostName)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	ctx := contxt.Background()

	if v, ok := obj.(*cmd.Cluster); ok {
		v.Tenant = tenant
		_, err := restcl.CmdV1().Cluster().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.Endpoint); ok {
		v.Tenant = tenant
		_, err := restcl.EndpointV1().Endpoint().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.LbPolicy); ok {
		v.Tenant = tenant
		_, err := restcl.LbPolicyV1().LbPolicy().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.Network); ok {
		v.Tenant = tenant
		_, err := restcl.NetworkV1().Network().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*cmd.Node); ok {
		v.Tenant = tenant
		_, err := restcl.CmdV1().Node().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.SecurityGroup); ok {
		v.Tenant = tenant
		_, err := restcl.SecurityGroupV1().SecurityGroup().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.Service); ok {
		v.Tenant = tenant
		_, err := restcl.ServiceV1().Service().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.Sgpolicy); ok {
		v.Tenant = tenant
		_, err := restcl.SgpolicyV1().Sgpolicy().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*cmd.SmartNIC); ok {
		v.Tenant = tenant
		_, err := restcl.CmdV1().SmartNIC().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.Tenant); ok {
		v.Tenant = tenant
		_, err := restcl.TenantV1().Tenant().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	return httpPost(url, obj)
}

func restPut(url, tenant string, obj interface{}) error {
	log.Debugf("put url: %s", url)

	urlStrs := strings.Split(url, "/")
	if len(urlStrs) < 3 {
		return fmt.Errorf("invalid url: '%s'", url)
	}
	hostName := strings.Join(urlStrs[:3], "/")

	restcl, err := apiclient.NewRestAPIClient(hostName)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	ctx := contxt.Background()

	if v, ok := obj.(*cmd.Cluster); ok {
		v.Tenant = tenant
		_, err := restcl.CmdV1().Cluster().Update(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.Endpoint); ok {
		v.Tenant = tenant
		_, err := restcl.EndpointV1().Endpoint().Update(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.LbPolicy); ok {
		v.Tenant = tenant
		_, err := restcl.LbPolicyV1().LbPolicy().Update(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.Network); ok {
		v.Tenant = tenant
		_, err := restcl.NetworkV1().Network().Update(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*cmd.Node); ok {
		v.Tenant = tenant
		_, err := restcl.CmdV1().Node().Update(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.SecurityGroup); ok {
		v.Tenant = tenant
		_, err := restcl.SecurityGroupV1().SecurityGroup().Update(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.Service); ok {
		v.Tenant = tenant
		_, err := restcl.ServiceV1().Service().Update(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.Sgpolicy); ok {
		v.Tenant = tenant
		_, err := restcl.SgpolicyV1().Sgpolicy().Update(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*cmd.SmartNIC); ok {
		v.Tenant = tenant
		_, err := restcl.CmdV1().SmartNIC().Update(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.Tenant); ok {
		v.Tenant = tenant
		_, err := restcl.TenantV1().Tenant().Update(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	return httpPost(url, obj)
}

var objOrder = []string{

	"cluster",

	"endpoint",

	"lbPolicy",

	"network",

	"node",

	"permission",

	"role",

	"securityGroup",

	"service",

	"sgpolicy",

	"smartNIC",

	"tenant",

	"user",
}
