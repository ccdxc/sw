package impl

import (
	"fmt"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	vldtor "github.com/pensando/sw/venice/utils/apigen/validators"
	"github.com/pensando/sw/venice/utils/log"
)

type clHooks struct {
	logger log.Logger
}

// errInvalidHostConfig returns error associated with invalid hostname
func (cl *clHooks) errInvalidHostConfig(host string) error {
	return fmt.Errorf("mis-configured host policy, invalid hostname: %s", host)
}

// errInvalidMacConfig returns error associated with invalid mac-address
func (cl *clHooks) errInvalidMacConfig(mac string) error {
	return fmt.Errorf("mis-configured host policy, invalid mac address: %s", mac)
}

// errInvalidTenantConfig returns error associated with invalid tenant
func (cl *clHooks) errInvalidTenantConfig() error {
	return fmt.Errorf("invalid config, tenant should be empty")
}

// Validate the Host config
func (cl *clHooks) validateHostConfig(i interface{}, ver string, ignStatus bool) []error {
	var err []error
	obj, ok := i.(cluster.Host)
	if !ok {
		return []error{fmt.Errorf("incorrect object type, expected host object")}
	}

	// validate the host object name
	if vldtor.HostAddr(obj.Name) == false {
		cl.logger.Errorf("Invalid host: %s", obj.Name)
		err = append(err, cl.errInvalidHostConfig(obj.Name))
	}

	// validate tenant
	if obj.Tenant != "" || len(obj.Tenant) != 0 {
		err = append(err, cl.errInvalidTenantConfig())
	}

	// validate the mac address in the interface spec
	for mackey, intf := range obj.Spec.Interfaces {
		if vldtor.MacAddr(mackey) == false {
			cl.logger.Errorf("Invalid mac key: %s", mackey)
			err = append(err, cl.errInvalidMacConfig(mackey))
		}
		for _, mac := range intf.MacAddrs {
			if vldtor.MacAddr(mac) == false {
				cl.logger.Errorf("Invalid mac addr: %s", mac)
				err = append(err, cl.errInvalidMacConfig(mac))
			}
		}
	}

	return err
}

// Validate the Node config
func (cl *clHooks) validateNodeConfig(i interface{}, ver string, ignStatus bool) []error {
	var err []error
	obj, ok := i.(cluster.Node)
	if !ok {
		return []error{fmt.Errorf("incorrect object type, expected node object")}
	}

	// validate tenant
	if obj.Tenant != "" || len(obj.Tenant) != 0 {
		err = append(err, cl.errInvalidTenantConfig())
	}

	return err
}

// Validate the Cluster config
func (cl *clHooks) validateClusterConfig(i interface{}, ver string, ignStatus bool) []error {
	var err []error
	obj, ok := i.(cluster.Cluster)
	if !ok {
		return []error{fmt.Errorf("incorrect object type, expected cluster object")}
	}

	// validate tenant
	if obj.Tenant != "" || len(obj.Tenant) != 0 {
		err = append(err, cl.errInvalidTenantConfig())
	}

	return err
}

func registerClusterHooks(svc apiserver.Service, logger log.Logger) {
	r := clHooks{}
	r.logger = logger.WithContext("Service", "ClusterHooks")
	logger.Log("msg", "registering Hooks for cluster apigroup")
	svc.GetCrudService("Host", apiserver.CreateOper).GetRequestType().WithValidate(r.validateHostConfig)
	svc.GetCrudService("Host", apiserver.UpdateOper).GetRequestType().WithValidate(r.validateHostConfig)
	svc.GetCrudService("Node", apiserver.CreateOper).GetRequestType().WithValidate(r.validateNodeConfig)
	svc.GetCrudService("Node", apiserver.UpdateOper).GetRequestType().WithValidate(r.validateNodeConfig)
	svc.GetCrudService("Cluster", apiserver.CreateOper).GetRequestType().WithValidate(r.validateClusterConfig)
	svc.GetCrudService("Cluster", apiserver.UpdateOper).GetRequestType().WithValidate(r.validateClusterConfig)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("cluster.ClusterV1", registerClusterHooks)
}
