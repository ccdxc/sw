package impl

import (
	"context"
	"errors"
	"fmt"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	vldtor "github.com/pensando/sw/venice/utils/apigen/validators"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

type clusterHooks struct {
	logger log.Logger
}

// errInvalidHostConfig returns error associated with invalid hostname
func (cl *clusterHooks) errInvalidHostConfig(host string) error {
	return fmt.Errorf("mis-configured host policy, invalid hostname: %s", host)
}

// errInvalidMacConfig returns error associated with invalid mac-address
func (cl *clusterHooks) errInvalidMacConfig(mac string) error {
	return fmt.Errorf("mis-configured host policy, invalid mac address: %s", mac)
}

// errInvalidTenantConfig returns error associated with invalid tenant
func (cl *clusterHooks) errInvalidTenantConfig() error {
	return fmt.Errorf("invalid config, tenant should be empty")
}

// Validate the Host config
func (cl *clusterHooks) validateHostConfig(i interface{}, ver string, ignStatus bool) []error {
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
func (cl *clusterHooks) validateNodeConfig(i interface{}, ver string, ignStatus bool) []error {
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
func (cl *clusterHooks) validateClusterConfig(i interface{}, ver string, ignStatus bool) []error {
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

// createDefaultRoles is a pre-commit hook for tenant create operation that creates default roles when a tenant is created
func (cl *clusterHooks) createDefaultRoles(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.Errorf("API server hook to create default roles called for invalid object type [%#v]", i)
		return i, false, errors.New("invalid input type")
	}
	if oper != apiserver.CreateOper {
		cl.logger.Errorf("API server hook to create default roles called for invalid API Operation [%s]", oper)
		return i, false, errors.New("invalid input type")
	}
	cl.logger.Debugf("API server hook called to create default roles for tenant [%v]", r.Name)

	allowedTenant := r.GetName()
	// if "default" tenant then give permissions to all tenants
	if r.GetName() == globals.DefaultTenant {
		allowedTenant = authz.ResourceTenantAll
	}
	// create tenant admin role
	adminRole := login.NewRole(globals.AdminRole, r.GetName(), login.NewPermission(
		allowedTenant,
		authz.ResourceGroupAll,
		auth.Permission_AllResourceKinds.String(),
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_ALL_ACTIONS.String()))
	// set version
	apiSrv := apisrvpkg.MustGetAPIServer()
	adminRole.APIVersion = apiSrv.GetVersion()
	adminRole.SelfLink = adminRole.MakeURI("configs", adminRole.APIVersion, "auth")
	if err := txn.Create(adminRole.MakeKey("auth"), adminRole); err != nil {
		return r, false, err
	}
	return r, true, nil
}

// deleteDefaultRoles is a pre-commit hook for tenant delete operation that deletes default roles when a tenant is deleted
func (cl *clusterHooks) deleteDefaultRoles(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.Errorf("API server hook to delete default roles called for invalid object type [%#v]", i)
		return i, false, errors.New("invalid input type")
	}
	if oper != apiserver.DeleteOper {
		cl.logger.Errorf("API server hook to delete default roles called for invalid API Operation [%s]", oper)
		return i, false, errors.New("invalid input type")
	}
	cl.logger.Debugf("API server hook called to delete default roles for tenant [%v]", r.Name)

	allowedTenant := r.GetName()

	// get role key
	adminRoleKey := login.NewRole(globals.AdminRole, r.GetName(), login.NewPermission(
		allowedTenant,
		authz.ResourceGroupAll,
		auth.Permission_AllResourceKinds.String(),
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_ALL_ACTIONS.String())).MakeKey("auth")
	if err := txn.Delete(adminRoleKey); err != nil {
		cl.logger.Errorf("Error adding delete admin role to transaction, Err: %v", err)
		return r, false, err
	}
	return r, true, nil
}

// checkAuthBootstrapFlag is a pre-commit hook for cluster create/update operation that makes sure that bootstrap flag is set to true only once for cluster update
func (cl *clusterHooks) checkAuthBootstrapFlag(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Cluster)
	if !ok {
		cl.logger.Errorf("API server hook to check bootstrap flag called for invalid object type [%#v]", i)
		return i, false, errors.New("invalid input type")
	}
	cl.logger.Infof("API server hook called to check bootstrap flag for cluster")

	switch oper {
	case apiserver.CreateOper:
		// always set Bootstrapped flag to false when cluster is created
		r.Status.AuthBootstrapped = false
		return r, true, nil
	case apiserver.UpdateOper:
		cur := &cluster.Cluster{}
		if err := kv.Get(ctx, key, cur); err != nil {
			cl.logger.Errorf("Error getting cluster with key [%s] in API server checkAuthBootstrapFlag pre-commit hook for create/update cluster", key)
			return r, false, err
		}
		if cur.Status.AuthBootstrapped {
			r.Status.AuthBootstrapped = true
		}
		if r.Status.AuthBootstrapped {
			cl.logger.Infof("Cluster bootstrap flag will now be set and locked down")
		}
		// Add a comparator for CAS
		cl.logger.Infof("set the comparator version for [%s] as [%s]", key, cur.ResourceVersion)
		txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", cur.ResourceVersion))
		return r, true, nil
	default:
		cl.logger.Errorf("API server hook to check bootstrap flag called for invalid API Operation [%s]", oper)
		return i, false, errors.New("invalid input type")
	}
}

// setAuthBootstrapFlag is a pre-commit hook to set bootstrap flag
func (cl *clusterHooks) setAuthBootstrapFlag(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	_, ok := i.(cluster.ClusterAuthBootstrapRequest)
	if !ok {
		cl.logger.Errorf("API server hook to check bootstrap flag called for invalid object type [%#v]", i)
		return i, false, errors.New("invalid input type")
	}
	cl.logger.Infof("API server hook called to set bootstrap flag")

	cur := &cluster.Cluster{}
	if err := kv.ConsistentUpdate(ctx, key, cur, func(oldObj runtime.Object) (runtime.Object, error) {
		clusterObj, ok := oldObj.(*cluster.Cluster)
		if !ok {
			return oldObj, errors.New("invalid input type")
		}
		clusterObj.Status.AuthBootstrapped = true
		return clusterObj, nil
	}); err != nil {
		cl.logger.Errorf("Error setting bootstrap flag: %v", err)
		return nil, false, err
	}
	cl.logger.Infof("Cluster bootstrap flag is set and locked down")
	return *cur, false, nil
}

func registerClusterHooks(svc apiserver.Service, logger log.Logger) {
	r := clusterHooks{}
	r.logger = logger.WithContext("Service", "ClusterHooks")
	logger.Log("msg", "registering Hooks for cluster apigroup")
	svc.GetCrudService("Host", apiserver.CreateOper).GetRequestType().WithValidate(r.validateHostConfig)
	svc.GetCrudService("Host", apiserver.UpdateOper).GetRequestType().WithValidate(r.validateHostConfig)
	svc.GetCrudService("Node", apiserver.CreateOper).GetRequestType().WithValidate(r.validateNodeConfig)
	svc.GetCrudService("Node", apiserver.UpdateOper).GetRequestType().WithValidate(r.validateNodeConfig)
	svc.GetCrudService("Cluster", apiserver.CreateOper).WithPreCommitHook(r.checkAuthBootstrapFlag).GetRequestType().WithValidate(r.validateClusterConfig)
	svc.GetCrudService("Cluster", apiserver.UpdateOper).WithPreCommitHook(r.checkAuthBootstrapFlag).GetRequestType().WithValidate(r.validateClusterConfig)
	// hook to set bootstrap flag
	svc.GetMethod("AuthBootstrapComplete").WithPreCommitHook(r.setAuthBootstrapFlag)
	// register hook to create default roles
	svc.GetCrudService("Tenant", apiserver.CreateOper).WithPreCommitHook(r.createDefaultRoles)
	svc.GetCrudService("Tenant", apiserver.DeleteOper).WithPreCommitHook(r.deleteDefaultRoles)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("cluster.ClusterV1", registerClusterHooks)
}
