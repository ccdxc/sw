// (c) Pensando Systems, Inc.
// This is a generated file, please do not hand edit !!

package tserver

import (
	"context"
	"path"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"

	"github.com/pensando/sw/venice/cli/api"
)

// GetClusterByName is
func GetClusterByName(objName string) (*cluster.Cluster, error) {
	obj, err := GetObjByName("cluster", objName)
	if err != nil {
		return nil, err
	}
	clusterObj := obj.(*cluster.Cluster)
	return clusterObj, nil
}

// GetEndpointByName is
func GetEndpointByName(objName string) (*workload.Endpoint, error) {
	obj, err := GetObjByName("endpoint", objName)
	if err != nil {
		return nil, err
	}
	endpointObj := obj.(*workload.Endpoint)
	return endpointObj, nil
}

// GetLbPolicyByName is
func GetLbPolicyByName(objName string) (*network.LbPolicy, error) {
	obj, err := GetObjByName("lbPolicy", objName)
	if err != nil {
		return nil, err
	}
	lbPolicyObj := obj.(*network.LbPolicy)
	return lbPolicyObj, nil
}

// GetNetworkByName is
func GetNetworkByName(objName string) (*network.Network, error) {
	obj, err := GetObjByName("network", objName)
	if err != nil {
		return nil, err
	}
	networkObj := obj.(*network.Network)
	return networkObj, nil
}

// GetNodeByName is
func GetNodeByName(objName string) (*cluster.Node, error) {
	obj, err := GetObjByName("node", objName)
	if err != nil {
		return nil, err
	}
	nodeObj := obj.(*cluster.Node)
	return nodeObj, nil
}

// GetPermissionByName is
func GetPermissionByName(objName string) (*api.Permission, error) {
	obj, err := GetObjByName("permission", objName)
	if err != nil {
		return nil, err
	}
	permissionObj := obj.(*api.Permission)
	return permissionObj, nil
}

// GetRoleByName is
func GetRoleByName(objName string) (*api.Role, error) {
	obj, err := GetObjByName("role", objName)
	if err != nil {
		return nil, err
	}
	roleObj := obj.(*api.Role)
	return roleObj, nil
}

// GetSecurityGroupByName is
func GetSecurityGroupByName(objName string) (*security.SecurityGroup, error) {
	obj, err := GetObjByName("securityGroup", objName)
	if err != nil {
		return nil, err
	}
	securityGroupObj := obj.(*security.SecurityGroup)
	return securityGroupObj, nil
}

// GetServiceByName is
func GetServiceByName(objName string) (*network.Service, error) {
	obj, err := GetObjByName("service", objName)
	if err != nil {
		return nil, err
	}
	serviceObj := obj.(*network.Service)
	return serviceObj, nil
}

// GetSgpolicyByName is
func GetSgpolicyByName(objName string) (*security.Sgpolicy, error) {
	obj, err := GetObjByName("sgpolicy", objName)
	if err != nil {
		return nil, err
	}
	sgpolicyObj := obj.(*security.Sgpolicy)
	return sgpolicyObj, nil
}

// GetSmartNICByName is
func GetSmartNICByName(objName string) (*cluster.SmartNIC, error) {
	obj, err := GetObjByName("smartNIC", objName)
	if err != nil {
		return nil, err
	}
	smartNICObj := obj.(*cluster.SmartNIC)
	return smartNICObj, nil
}

// GetTenantByName is
func GetTenantByName(objName string) (*cluster.Tenant, error) {
	obj, err := GetObjByName("tenant", objName)
	if err != nil {
		return nil, err
	}
	tenantObj := obj.(*cluster.Tenant)
	return tenantObj, nil
}

// GetUserByName is
func GetUserByName(objName string) (*api.User, error) {
	obj, err := GetObjByName("user", objName)
	if err != nil {
		return nil, err
	}
	userObj := obj.(*api.User)
	return userObj, nil
}

// UpdateCluster is
func UpdateCluster(obj *cluster.Cluster) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["cluster"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}

// UpdateEndpoint is
func UpdateEndpoint(obj *workload.Endpoint) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["endpoint"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}

// UpdateLbPolicy is
func UpdateLbPolicy(obj *network.LbPolicy) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["lbPolicy"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}

// UpdateNetwork is
func UpdateNetwork(obj *network.Network) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["network"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}

// UpdateNode is
func UpdateNode(obj *cluster.Node) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["node"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}

// UpdatePermission is
func UpdatePermission(obj *api.Permission) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["permission"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}

// UpdateRole is
func UpdateRole(obj *api.Role) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["role"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}

// UpdateSecurityGroup is
func UpdateSecurityGroup(obj *security.SecurityGroup) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["securityGroup"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}

// UpdateService is
func UpdateService(obj *network.Service) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["service"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}

// UpdateSgpolicy is
func UpdateSgpolicy(obj *security.Sgpolicy) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["sgpolicy"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}

// UpdateSmartNIC is
func UpdateSmartNIC(obj *cluster.SmartNIC) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["smartNIC"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}

// UpdateTenant is
func UpdateTenant(obj *cluster.Tenant) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["tenant"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}

// UpdateUser is
func UpdateUser(obj *api.User) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["user"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}

func getEmptyObj(kind string) (obj interface{}, objList interface{}) {
	switch kind {

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
		return &cluster
	}

	if ol, ok := objList.(*workload.EndpointList); ok {
		endpoint := ol.Items[idx]
		return &endpoint
	}

	if ol, ok := objList.(*network.LbPolicyList); ok {
		lbPolicy := ol.Items[idx]
		return &lbPolicy
	}

	if ol, ok := objList.(*network.NetworkList); ok {
		network := ol.Items[idx]
		return &network
	}

	if ol, ok := objList.(*cluster.NodeList); ok {
		node := ol.Items[idx]
		return &node
	}

	if ol, ok := objList.(*api.PermissionList); ok {
		permission := ol.Items[idx]
		return &permission
	}

	if ol, ok := objList.(*api.RoleList); ok {
		role := ol.Items[idx]
		return &role
	}

	if ol, ok := objList.(*security.SecurityGroupList); ok {
		securityGroup := ol.Items[idx]
		return &securityGroup
	}

	if ol, ok := objList.(*network.ServiceList); ok {
		service := ol.Items[idx]
		return &service
	}

	if ol, ok := objList.(*security.SgpolicyList); ok {
		sgpolicy := ol.Items[idx]
		return &sgpolicy
	}

	if ol, ok := objList.(*cluster.SmartNICList); ok {
		smartNIC := ol.Items[idx]
		return &smartNIC
	}

	if ol, ok := objList.(*cluster.TenantList); ok {
		tenant := ol.Items[idx]
		return &tenant
	}

	if ol, ok := objList.(*api.UserList); ok {
		user := ol.Items[idx]
		return &user
	}

	return nil
}
