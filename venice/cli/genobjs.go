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
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/api/labels"
	loginctx "github.com/pensando/sw/api/login/context"

	api2 "github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cli/api"
	"github.com/pensando/sw/venice/cli/gen/pregen"
	"github.com/pensando/sw/venice/utils/ref"
)

func getObj(ctx *context) (obj interface{}, objList interface{}) {
	switch ctx.subcmd {

	case "cluster":
		return &cluster.Cluster{}, &cluster.ClusterList{}

	case "endpoint":
		return &workload.Endpoint{}, &workload.EndpointList{}

	case "lbPolicy":
		return &network.LbPolicy{}, &network.LbPolicyList{}

	case "network":
		return &network.Network{}, &network.NetworkList{}

	case "node":
		return &cluster.Node{}, &cluster.NodeList{}

	case "permission":
		return &api.Permission{}, &api.PermissionList{}

	case "role":
		return &api.Role{}, &api.RoleList{}

	case "securityGroup":
		return &security.SecurityGroup{}, &security.SecurityGroupList{}

	case "service":
		return &network.Service{}, &network.ServiceList{}

	case "sgpolicy":
		return &security.Sgpolicy{}, &security.SgpolicyList{}

	case "smartNIC":
		return &cluster.SmartNIC{}, &cluster.SmartNICList{}

	case "tenant":
		return &cluster.Tenant{}, &cluster.TenantList{}

	case "user":
		return &api.User{}, &api.UserList{}

	}
	return nil, nil
}

func getObjFromList(objList interface{}, idx int) interface{} {

	if ol, ok := objList.(*cluster.ClusterList); ok {
		cluster := ol.Items[idx]
		return cluster
	}

	if ol, ok := objList.(*workload.EndpointList); ok {
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

	if ol, ok := objList.(*cluster.NodeList); ok {
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

	if ol, ok := objList.(*security.SecurityGroupList); ok {
		securityGroup := ol.Items[idx]
		return securityGroup
	}

	if ol, ok := objList.(*network.ServiceList); ok {
		service := ol.Items[idx]
		return service
	}

	if ol, ok := objList.(*security.SgpolicyList); ok {
		sgpolicy := ol.Items[idx]
		return sgpolicy
	}

	if ol, ok := objList.(*cluster.SmartNICList); ok {
		smartNIC := ol.Items[idx]
		return smartNIC
	}

	if ol, ok := objList.(*cluster.TenantList); ok {
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

	if v, ok := obj.(*cluster.Cluster); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = cluster.ClusterStatus{}
	}

	if v, ok := obj.(*workload.Endpoint); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = workload.EndpointStatus{}
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

	if v, ok := obj.(*cluster.Node); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = cluster.NodeStatus{}
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

	if v, ok := obj.(*security.SecurityGroup); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = security.SecurityGroupStatus{}
	}

	if v, ok := obj.(*network.Service); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = network.ServiceStatus{}
	}

	if v, ok := obj.(*security.Sgpolicy); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = security.SgpolicyStatus{}
	}

	if v, ok := obj.(*cluster.SmartNIC); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = cluster.SmartNICStatus{}
	}

	if v, ok := obj.(*cluster.Tenant); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = cluster.TenantStatus{}
	}

	if v, ok := obj.(*api.User); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = api.UserStatus{}
	}

	return nil
}

func writeObj(obj interface{}, objmKvs, specKvs map[string]ref.FInfo) interface{} {

	if v, ok := obj.(*cluster.Cluster); ok {
		return writeClusterObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*workload.Endpoint); ok {
		return writeEndpointObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*network.LbPolicy); ok {
		return writeLbPolicyObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*network.Network); ok {
		return writeNetworkObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*cluster.Node); ok {
		return writeNodeObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*api.Permission); ok {
		return writePermissionObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*api.Role); ok {
		return writeRoleObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*security.SecurityGroup); ok {
		return writeSecurityGroupObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*network.Service); ok {
		return writeServiceObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*security.Sgpolicy); ok {
		return writeSgpolicyObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*cluster.SmartNIC); ok {
		return writeSmartNICObj(*v, objmKvs, specKvs)
	}

	if v, ok := obj.(*cluster.Tenant); ok {
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
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

	switch ctx.subcmd {

	case "cluster":
		clusterList := objList.(*cluster.ClusterList)
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
		endpointList := objList.(*workload.EndpointList)
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
		nodeList := objList.(*cluster.NodeList)
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
		securityGroupList := objList.(*security.SecurityGroupList)
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
		sgpolicyList := objList.(*security.SgpolicyList)
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
		smartNICList := objList.(*cluster.SmartNICList)
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
		tenantList := objList.(*cluster.TenantList)
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

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newTimestamp := new.(api2.Timestamp)

	return &newTimestamp
}

func writeHealthCheckSpecObj(obj network.HealthCheckSpec, specKvs map[string]ref.FInfo) *network.HealthCheckSpec {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newHealthCheckSpec := new.(network.HealthCheckSpec)

	return &newHealthCheckSpec
}

func writeNodeConditionObj(obj cluster.NodeCondition, specKvs map[string]ref.FInfo) *cluster.NodeCondition {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newNodeCondition := new.(cluster.NodeCondition)

	return &newNodeCondition
}

func writePortConditionObj(obj cluster.PortCondition, specKvs map[string]ref.FInfo) *cluster.PortCondition {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newPortCondition := new.(cluster.PortCondition)

	return &newPortCondition
}

func writeConditionStatusObj(obj cluster.ConditionStatus, specKvs map[string]ref.FInfo) *cluster.ConditionStatus {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newConditionStatus := new.(cluster.ConditionStatus)

	return &newConditionStatus
}

func writeSelectorObj(obj labels.Selector, specKvs map[string]ref.FInfo) *labels.Selector {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newSelector := new.(labels.Selector)

	return &newSelector
}

func writeRequirementObj(obj labels.Requirement, specKvs map[string]ref.FInfo) *labels.Requirement {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newRequirement := new.(labels.Requirement)

	return &newRequirement
}

func writeTLSServerPolicySpecObj(obj network.TLSServerPolicySpec, specKvs map[string]ref.FInfo) *network.TLSServerPolicySpec {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newTLSServerPolicySpec := new.(network.TLSServerPolicySpec)

	return &newTLSServerPolicySpec
}

func writeTLSClientPolicySpecObj(obj network.TLSClientPolicySpec, specKvs map[string]ref.FInfo) *network.TLSClientPolicySpec {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newTLSClientPolicySpec := new.(network.TLSClientPolicySpec)

	return &newTLSClientPolicySpec
}

func writeSGRuleObj(obj security.SGRule, specKvs map[string]ref.FInfo) *security.SGRule {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newSGRule := new.(security.SGRule)

	return &newSGRule
}

func writePortSpecObj(obj cluster.PortSpec, specKvs map[string]ref.FInfo) *cluster.PortSpec {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newPortSpec := new.(cluster.PortSpec)

	return &newPortSpec
}

func writePortStatusObj(obj cluster.PortStatus, specKvs map[string]ref.FInfo) *cluster.PortStatus {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newPortStatus := new.(cluster.PortStatus)

	return &newPortStatus
}

func writeSmartNICConditionObj(obj cluster.SmartNICCondition, specKvs map[string]ref.FInfo) *cluster.SmartNICCondition {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newSmartNICCondition := new.(cluster.SmartNICCondition)

	return &newSmartNICCondition
}

func writeUserAuditLogObj(obj api.UserAuditLog, specKvs map[string]ref.FInfo) *api.UserAuditLog {

	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	newUserAuditLog := new.(api.UserAuditLog)

	return &newUserAuditLog
}

func writeClusterObj(obj cluster.Cluster, metaKvs, specKvs map[string]ref.FInfo) *cluster.Cluster {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newCluster := &cluster.Cluster{
		TypeMeta:   swapi.TypeMeta{Kind: "cluster"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(cluster.ClusterSpec),
	}
	return newCluster
}

func writeEndpointObj(obj workload.Endpoint, metaKvs, specKvs map[string]ref.FInfo) *workload.Endpoint {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newEndpoint := &workload.Endpoint{
		TypeMeta:   swapi.TypeMeta{Kind: "endpoint"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(workload.EndpointSpec),
	}
	return newEndpoint
}

func writeLbPolicyObj(obj network.LbPolicy, metaKvs, specKvs map[string]ref.FInfo) *network.LbPolicy {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

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
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newNetwork := &network.Network{
		TypeMeta:   swapi.TypeMeta{Kind: "network"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(network.NetworkSpec),
	}
	return newNetwork
}

func writeNodeObj(obj cluster.Node, metaKvs, specKvs map[string]ref.FInfo) *cluster.Node {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newNode := &cluster.Node{
		TypeMeta:   swapi.TypeMeta{Kind: "node"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(cluster.NodeSpec),
	}
	return newNode
}

func writePermissionObj(obj api.Permission, metaKvs, specKvs map[string]ref.FInfo) *api.Permission {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

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
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newRole := &api.Role{
		TypeMeta:   swapi.TypeMeta{Kind: "role"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(api.RoleSpec),
	}
	return newRole
}

func writeSecurityGroupObj(obj security.SecurityGroup, metaKvs, specKvs map[string]ref.FInfo) *security.SecurityGroup {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newSecurityGroup := &security.SecurityGroup{
		TypeMeta:   swapi.TypeMeta{Kind: "securityGroup"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(security.SecurityGroupSpec),
	}
	return newSecurityGroup
}

func writeServiceObj(obj network.Service, metaKvs, specKvs map[string]ref.FInfo) *network.Service {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newService := &network.Service{
		TypeMeta:   swapi.TypeMeta{Kind: "service"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(network.ServiceSpec),
	}
	return newService
}

func writeSgpolicyObj(obj security.Sgpolicy, metaKvs, specKvs map[string]ref.FInfo) *security.Sgpolicy {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newSgpolicy := &security.Sgpolicy{
		TypeMeta:   swapi.TypeMeta{Kind: "sgpolicy"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(security.SgpolicySpec),
	}
	return newSgpolicy
}

func writeSmartNICObj(obj cluster.SmartNIC, metaKvs, specKvs map[string]ref.FInfo) *cluster.SmartNIC {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newSmartNIC := &cluster.SmartNIC{
		TypeMeta:   swapi.TypeMeta{Kind: "smartNIC"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(cluster.SmartNICSpec),
	}
	return newSmartNIC
}

func writeTenantObj(obj cluster.Tenant, metaKvs, specKvs map[string]ref.FInfo) *cluster.Tenant {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	newTenant := &cluster.Tenant{
		TypeMeta:   swapi.TypeMeta{Kind: "tenant"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.(cluster.TenantSpec),
	}
	return newTenant
}

func writeUserObj(obj api.User, metaKvs, specKvs map[string]ref.FInfo) *api.User {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true, CustomParsers: api.CustomParsers}

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

func createEndpointFromBytes(ctx *context, inp string) error {
	endpoint := &workload.Endpoint{}
	if err := json.Unmarshal([]byte(inp), endpoint); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	endpoint.Tenant = ctx.tenant
	if err := postObj(ctx, endpoint); err != nil {
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
	if err := postObj(ctx, lbPolicy); err != nil {
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
	if err := postObj(ctx, network); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createNodeFromBytes(ctx *context, inp string) error {
	node := &cluster.Node{}
	if err := json.Unmarshal([]byte(inp), node); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	node.Tenant = ctx.tenant
	if err := postObj(ctx, node); err != nil {
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
	if err := postObj(ctx, permission); err != nil {
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
	if err := postObj(ctx, role); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createSecurityGroupFromBytes(ctx *context, inp string) error {
	securityGroup := &security.SecurityGroup{}
	if err := json.Unmarshal([]byte(inp), securityGroup); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	securityGroup.Tenant = ctx.tenant
	if err := postObj(ctx, securityGroup); err != nil {
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
	if err := postObj(ctx, service); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createSgpolicyFromBytes(ctx *context, inp string) error {
	sgpolicy := &security.Sgpolicy{}
	if err := json.Unmarshal([]byte(inp), sgpolicy); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	sgpolicy.Tenant = ctx.tenant
	if err := postObj(ctx, sgpolicy); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createSmartNICFromBytes(ctx *context, inp string) error {
	smartNIC := &cluster.SmartNIC{}
	if err := json.Unmarshal([]byte(inp), smartNIC); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	smartNIC.Tenant = ctx.tenant
	if err := postObj(ctx, smartNIC); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func createTenantFromBytes(ctx *context, inp string) error {
	tenant := &cluster.Tenant{}
	if err := json.Unmarshal([]byte(inp), tenant); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	tenant.Tenant = ctx.tenant
	if err := postObj(ctx, tenant); err != nil {
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
	if err := postObj(ctx, user); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}

func updateLabel(obj interface{}, newLabels map[string]string) error {

	if o, ok := obj.(*cluster.Cluster); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*workload.Endpoint); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*network.LbPolicy); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*network.Network); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*cluster.Node); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*api.Permission); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*api.Role); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*security.SecurityGroup); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*network.Service); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*security.Sgpolicy); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*cluster.SmartNIC); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*cluster.Tenant); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	if o, ok := obj.(*api.User); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}

	return fmt.Errorf("unknown object")
}

func restGet(url, tenant, token string, obj interface{}) error {
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
	ctx := loginctx.NewContextWithAuthzHeader(contxt.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Cluster); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.ClusterV1().Cluster().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*cluster.ClusterList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.ClusterV1().Cluster().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*workload.Endpoint); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.WorkloadV1().Endpoint().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*workload.EndpointList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.WorkloadV1().Endpoint().List(ctx, &opts)
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
		nv, err := restcl.NetworkV1().LbPolicy().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*network.LbPolicyList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.NetworkV1().LbPolicy().List(ctx, &opts)
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

	if v, ok := obj.(*cluster.Node); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.ClusterV1().Node().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*cluster.NodeList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.ClusterV1().Node().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*security.SecurityGroup); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.SecurityV1().SecurityGroup().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*security.SecurityGroupList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.SecurityV1().SecurityGroup().List(ctx, &opts)
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
		nv, err := restcl.NetworkV1().Service().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*network.ServiceList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.NetworkV1().Service().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*security.Sgpolicy); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.SecurityV1().Sgpolicy().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*security.SgpolicyList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.SecurityV1().Sgpolicy().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*cluster.SmartNIC); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.ClusterV1().SmartNIC().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*cluster.SmartNICList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.ClusterV1().SmartNIC().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	if v, ok := obj.(*cluster.Tenant); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.ClusterV1().Tenant().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*cluster.TenantList); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.ClusterV1().Tenant().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}

	return httpGet(url, obj)
}

func restDelete(objKind, url, tenant, token string) error {
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
	ctx := loginctx.NewContextWithAuthzHeader(contxt.Background(), "Bearer "+token)

	if objKind == "cluster" {
		objm := swapi.ObjectMeta{}
		objm.Name = objName
		objm.Tenant = tenant
		obj, err := restcl.ClusterV1().Cluster().Delete(ctx, &objm)
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
		obj, err := restcl.WorkloadV1().Endpoint().Delete(ctx, &objm)
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
		obj, err := restcl.NetworkV1().LbPolicy().Delete(ctx, &objm)
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
		obj, err := restcl.ClusterV1().Node().Delete(ctx, &objm)
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
		obj, err := restcl.SecurityV1().SecurityGroup().Delete(ctx, &objm)
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
		obj, err := restcl.NetworkV1().Service().Delete(ctx, &objm)
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
		obj, err := restcl.SecurityV1().Sgpolicy().Delete(ctx, &objm)
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
		obj, err := restcl.ClusterV1().SmartNIC().Delete(ctx, &objm)
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
		obj, err := restcl.ClusterV1().Tenant().Delete(ctx, &objm)
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

func restPost(url, tenant, token string, obj interface{}) error {
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
	ctx := loginctx.NewContextWithAuthzHeader(contxt.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Cluster); ok {
		v.Tenant = tenant
		_, err := restcl.ClusterV1().Cluster().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*workload.Endpoint); ok {
		v.Tenant = tenant
		_, err := restcl.WorkloadV1().Endpoint().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.LbPolicy); ok {
		v.Tenant = tenant
		_, err := restcl.NetworkV1().LbPolicy().Create(ctx, v)
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

	if v, ok := obj.(*cluster.Node); ok {
		v.Tenant = tenant
		_, err := restcl.ClusterV1().Node().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*security.SecurityGroup); ok {
		v.Tenant = tenant
		_, err := restcl.SecurityV1().SecurityGroup().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*network.Service); ok {
		v.Tenant = tenant
		_, err := restcl.NetworkV1().Service().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*security.Sgpolicy); ok {
		v.Tenant = tenant
		_, err := restcl.SecurityV1().Sgpolicy().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*cluster.SmartNIC); ok {
		v.Tenant = tenant
		_, err := restcl.ClusterV1().SmartNIC().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	if v, ok := obj.(*cluster.Tenant); ok {
		v.Tenant = tenant
		_, err := restcl.ClusterV1().Tenant().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}

	return httpPost(url, obj)
}

func restPut(url, tenant, token string, obj interface{}) error {
	log.Debugf("put url: %s", url)

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
	ctx := loginctx.NewContextWithAuthzHeader(contxt.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Cluster); ok {
		v.Tenant = tenant
		v.Name = objName
		nv, err := restcl.ClusterV1().Cluster().Update(ctx, v)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}

	if v, ok := obj.(*workload.Endpoint); ok {
		v.Tenant = tenant
		v.Name = objName
		nv, err := restcl.WorkloadV1().Endpoint().Update(ctx, v)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}

	if v, ok := obj.(*network.LbPolicy); ok {
		v.Tenant = tenant
		v.Name = objName
		nv, err := restcl.NetworkV1().LbPolicy().Update(ctx, v)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}

	if v, ok := obj.(*network.Network); ok {
		v.Tenant = tenant
		v.Name = objName
		nv, err := restcl.NetworkV1().Network().Update(ctx, v)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}

	if v, ok := obj.(*cluster.Node); ok {
		v.Tenant = tenant
		v.Name = objName
		nv, err := restcl.ClusterV1().Node().Update(ctx, v)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}

	if v, ok := obj.(*security.SecurityGroup); ok {
		v.Tenant = tenant
		v.Name = objName
		nv, err := restcl.SecurityV1().SecurityGroup().Update(ctx, v)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}

	if v, ok := obj.(*network.Service); ok {
		v.Tenant = tenant
		v.Name = objName
		nv, err := restcl.NetworkV1().Service().Update(ctx, v)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}

	if v, ok := obj.(*security.Sgpolicy); ok {
		v.Tenant = tenant
		v.Name = objName
		nv, err := restcl.SecurityV1().Sgpolicy().Update(ctx, v)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}

	if v, ok := obj.(*cluster.SmartNIC); ok {
		v.Tenant = tenant
		v.Name = objName
		nv, err := restcl.ClusterV1().SmartNIC().Update(ctx, v)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}

	if v, ok := obj.(*cluster.Tenant); ok {
		v.Tenant = tenant
		v.Name = objName
		nv, err := restcl.ClusterV1().Tenant().Update(ctx, v)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}

	return httpPut(url, obj)
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
