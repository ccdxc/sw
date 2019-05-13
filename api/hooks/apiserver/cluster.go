package impl

import (
	"context"
	"crypto/tls"
	"errors"
	"fmt"
	"strconv"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

type clusterHooks struct {
	logger log.Logger
}

// errInvalidTenantConfig returns error associated with invalid tenant
func (cl *clusterHooks) errInvalidTenantConfig() error {
	return fmt.Errorf("invalid config, tenant should be empty")
}

// errInvalidNamespaceConfig returns error associated with invalid namespace
func (cl *clusterHooks) errInvalidNamespaceConfig() error {
	return fmt.Errorf("invalid config, namespace should be empty")
}

// Validate the Node config
func (cl *clusterHooks) validateNodeConfig(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
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
func (cl *clusterHooks) validateClusterConfig(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
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

// deleteDefaultTelemetryPolicies is a pre-commit hook for tenant create operation that deletes the auto added telemetry policies attached to the tenant
func (cl *clusterHooks) deleteDefaultTelemetryPolicies(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.Errorf("API server hook to create default roles called for invalid object type [%#v]", i)
		return i, false, errors.New("invalid input type")
	}
	statpol := monitoring.StatsPolicy{ObjectMeta: api.ObjectMeta{Name: r.Name, Tenant: r.Name}}
	statkey := statpol.MakeKey("monitoring")
	fwlogpol := monitoring.FwlogPolicy{ObjectMeta: api.ObjectMeta{Name: r.Name, Tenant: r.Name}}
	fwlogkey := fwlogpol.MakeKey("monitoring")
	if err := txn.Delete(statkey); err != nil {
		cl.logger.Errorf("txn delete got error (%v)\n", err)
		return i, false, err
	}
	if err := txn.Delete(fwlogkey); err != nil {
		cl.logger.Errorf("txn delete got error (%v)\n", err)
		return i, false, err
	}
	return r, true, nil
}

// Validate the given tenant config
func (cl *clusterHooks) validateTenantConfig(tenantConfig cluster.Tenant) error {
	tenantName := tenantConfig.GetName()

	if len(tenantName) > api.MaxTenantNameLen {
		return fmt.Errorf("tenant name too long (max %v chars)", api.MaxTenantNameLen)
	}

	// allow only lower case characters in tenant name
	if !api.TenantNameRe.Match([]byte(tenantName)) {
		return errors.New("tenant name does not meet naming requirements")
	}

	return nil
}

// createFirewallProfile is a pre-commit hook to creates default firewall policy when a tenant is created
func (cl *clusterHooks) createFirewallProfile(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.ErrorLog("method", "createFirewallProfile", "msg", fmt.Sprintf("API server hook to create default firewall profile called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	fwp := &security.FirewallProfile{}
	fwp.Defaults("all")
	apiSrv := apisrvpkg.MustGetAPIServer()
	fwp.APIVersion = apiSrv.GetVersion()
	fwp.SelfLink = fwp.MakeURI("configs", fwp.APIVersion, string(apiclient.GroupSecurity))
	fwp.Name = "default"
	fwp.Tenant = r.Name
	fwp.GenerationID = "1"
	fwp.UUID = uuid.NewV4().String()
	ts, err := types.TimestampProto(time.Now())
	if err != nil {
		return i, true, err
	}
	fwp.CreationTime, fwp.ModTime = api.Timestamp{Timestamp: *ts}, api.Timestamp{Timestamp: *ts}
	fwk := fwp.MakeKey(string(apiclient.GroupSecurity))
	err = txn.Create(fwk, fwp)
	if err != nil {
		return r, true, errors.New("adding create operation to transaction failed")
	}
	return r, true, nil
}

// deleteFirewallProfile is a pre-commit hook to delete default firewall policy when a tenant is deleted
func (cl *clusterHooks) deleteFirewallProfile(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.ErrorLog("method", "deleteFirewallProfile", "msg", fmt.Sprintf("API server hook to delete default firewall profile called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	fwp := &security.FirewallProfile{}
	fwp.Defaults("all")
	apiSrv := apisrvpkg.MustGetAPIServer()
	fwp.APIVersion = apiSrv.GetVersion()
	fwp.SelfLink = fwp.MakeURI("configs", fwp.APIVersion, string(apiclient.GroupSecurity))
	fwp.Name = "default"
	fwp.Tenant = r.Name
	fwk := fwp.MakeKey(string(apiclient.GroupSecurity))
	err := txn.Delete(fwk)
	if err != nil {
		return r, true, errors.New("adding delete operation to transaction failed")
	}
	return r, true, nil
}

// createDefaultVirtualRouter is a pre-commit hook to creates default VRF when a tenant is created
func (cl *clusterHooks) createDefaultVirtualRouter(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.ErrorLog("method", "createDefaultVirtualRouter", "msg", fmt.Sprintf("API server hook to create default VRF called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	vrf := &network.VirtualRouter{}
	vrf.Defaults("all")
	apiSrv := apisrvpkg.MustGetAPIServer()
	vrf.APIVersion = apiSrv.GetVersion()
	vrf.SelfLink = vrf.MakeURI("configs", vrf.APIVersion, string(apiclient.GroupNetwork))
	vrf.Name = "default"
	vrf.Tenant = r.Name
	vrf.Namespace = r.Namespace
	vrf.GenerationID = "1"
	vrf.UUID = uuid.NewV4().String()
	ts, err := types.TimestampProto(time.Now())
	if err != nil {
		return i, true, err
	}
	vrf.CreationTime, vrf.ModTime = api.Timestamp{Timestamp: *ts}, api.Timestamp{Timestamp: *ts}
	vrfk := vrf.MakeKey(string(apiclient.GroupNetwork))
	err = txn.Create(vrfk, vrf)
	if err != nil {
		return r, true, errors.New("adding create operation to transaction failed")
	}
	return r, true, nil
}

// deleteDefaultVirtualRouter is a pre-commit hook to delete default VRF when a tenant is deleted
func (cl *clusterHooks) deleteDefaultVirtualRouter(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.ErrorLog("method", "deleteDefaultVirtualRouter", "msg", fmt.Sprintf("API server hook to delete default firewall profile called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	vrf := &network.VirtualRouter{}
	vrf.Defaults("all")
	apiSrv := apisrvpkg.MustGetAPIServer()
	vrf.APIVersion = apiSrv.GetVersion()
	vrf.SelfLink = vrf.MakeURI("configs", vrf.APIVersion, string(apiclient.GroupNetwork))
	vrf.Name = "default"
	vrf.Tenant = r.Name
	vrf.Namespace = r.Namespace
	vrfk := vrf.MakeKey(string(apiclient.GroupNetwork))
	err := txn.Delete(vrfk)
	if err != nil {
		return r, true, errors.New("adding delete operation to transaction failed")
	}
	return r, true, nil
}

// Validate the tenant object
func (cl *clusterHooks) validateTenant(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
	r, ok := i.(cluster.Tenant)
	if !ok {
		return []error{errors.New("invalid intput type")}
	}
	apiSrv := apisrvpkg.MustGetAPIServer()
	flags := apiSrv.RuntimeFlags()
	if !flags.AllowMultiTenant && r.Name != globals.DefaultTenant {
		return []error{errors.New("only tenant default allowed")}
	}
	return nil
}

// createDefaultRoles is a pre-commit hook for tenant create operation that creates default roles when a tenant is created
func (cl *clusterHooks) createDefaultRoles(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.ErrorLog("method", "createDefaultRoles", "msg", fmt.Sprintf("API server hook to create default roles called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	if oper != apiintf.CreateOper {
		cl.logger.ErrorLog("method", "createDefaultRoles", "msg", fmt.Sprintf("API server hook to create default roles called for invalid API Operation [%s]", oper))
		return i, true, errors.New("invalid input type")
	}
	cl.logger.DebugLog("method", "createDefaultRoles", "msg", fmt.Sprintf("API server hook called to create default roles for tenant [%v]", r.Name))

	if err := cl.validateTenantConfig(r); err != nil {
		return i, true, err
	}

	allowedTenant := r.GetName()
	// if "default" tenant then give permissions to all tenants
	if r.GetName() == globals.DefaultTenant {
		allowedTenant = authz.ResourceTenantAll
	}
	// create tenant admin role
	adminRole := login.NewRole(globals.AdminRole, r.GetName(), login.NewPermission(
		allowedTenant,
		authz.ResourceGroupAll,
		authz.ResourceKindAll,
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_AllActions.String()))
	// set version
	apiSrv := apisrvpkg.MustGetAPIServer()
	adminRole.APIVersion = apiSrv.GetVersion()
	adminRole.SelfLink = adminRole.MakeURI("configs", adminRole.APIVersion, "auth")
	if err := txn.Create(adminRole.MakeKey("auth"), adminRole); err != nil {
		return r, true, err
	}
	// create tenant admin role binding
	adminRoleBinding := login.NewRoleBinding(globals.AdminRoleBinding, r.GetName(), globals.AdminRole, "", "")
	adminRoleBinding.APIVersion = apiSrv.GetVersion()
	adminRoleBinding.SelfLink = adminRoleBinding.MakeURI("configs", adminRole.APIVersion, "auth")
	if err := txn.Create(adminRoleBinding.MakeKey("auth"), adminRoleBinding); err != nil {
		return r, true, err
	}
	return r, true, nil
}

// createDefaultAlertPolicy creates a default alert policy (ies) for the user. This is mainly to make the life easy for the user.
// so, we do not manage the life cycle of these objects. User can update/delete these objects as like any other objects that were created by user.
func (cl *clusterHooks) createDefaultAlertPolicy(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.ErrorLog("method", "createDefaultAlertPolicy", "msg", fmt.Sprintf("API server hook to create default alert policy called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	cl.logger.DebugLog("method", "createDefaultAlertPolicy", "msg", fmt.Sprintf("API server hook called to create default alert policy for tenant [%v]", r.Name))

	apiServer := apisrvpkg.MustGetAPIServer()
	ts, _ := types.TimestampProto(time.Now())

	// create event based alert policy that converts CRITICAL events to CRITICAL alerts
	alertPolicy := &monitoring.AlertPolicy{}

	// meta
	alertPolicy.Defaults("all")
	alertPolicy.APIVersion = apiServer.GetVersion()
	alertPolicy.Name = "default-event-based-alerts"
	alertPolicy.UUID = uuid.NewV4().String()
	alertPolicy.CreationTime = api.Timestamp{Timestamp: *ts}
	alertPolicy.ModTime = api.Timestamp{Timestamp: *ts}
	alertPolicy.Tenant = r.GetName()
	alertPolicy.Namespace = globals.DefaultNamespace
	alertPolicy.GenerationID = "1"
	alertPolicy.SelfLink = alertPolicy.MakeURI("configs", alertPolicy.APIVersion, string(apiclient.GroupMonitoring))

	// spec
	alertPolicy.Spec.Resource = "Event"
	alertPolicy.Spec.Severity = eventattrs.Severity_CRITICAL.String()
	alertPolicy.Spec.Enable = true
	alertPolicy.Spec.Requirements = []*fields.Requirement{
		{Key: "severity", Operator: "equals", Values: []string{eventattrs.Severity_CRITICAL.String()}},
	}

	if err := txn.Create(alertPolicy.MakeKey(string(apiclient.GroupMonitoring)), alertPolicy); err != nil {
		return r, true, err
	}

	return r, true, nil
}

// deleteDefaultRoles is a pre-commit hook for tenant delete operation that deletes default roles when a tenant is deleted
func (cl *clusterHooks) deleteDefaultRoles(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.ErrorLog("method", "deleteDefaultRoles", "msg", fmt.Sprintf("API server hook to delete default roles called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	if oper != apiintf.DeleteOper {
		cl.logger.ErrorLog("method", "deleteDefaultRoles", "msg", fmt.Sprintf("API server hook to delete default roles called for invalid API Operation [%s]", oper))
		return i, true, errors.New("invalid input type")
	}
	cl.logger.DebugLog("method", "deleteDefaultRoles", "msg", fmt.Sprintf("API server hook called to delete default roles for tenant [%v]", r.Name))

	allowedTenant := r.GetName()

	// get role key
	adminRoleKey := login.NewRole(globals.AdminRole, r.GetName(), login.NewPermission(
		allowedTenant,
		authz.ResourceGroupAll,
		authz.ResourceKindAll,
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_AllActions.String())).MakeKey("auth")
	if err := txn.Delete(adminRoleKey); err != nil {
		cl.logger.ErrorLog("method", "deleteDefaultRoles", "msg", "error adding delete admin role to transaction", "error", err)
		return r, true, err
	}
	// get admin role binding key
	adminRoleBindingKey := login.NewRoleBinding(globals.AdminRoleBinding, r.GetName(), globals.AdminRole, "", "").MakeKey("auth")
	if err := txn.Delete(adminRoleBindingKey); err != nil {
		cl.logger.ErrorLog("method", "deleteDefaultRoles", "msg", "error adding delete admin role binding to transaction", "error", err)
		return r, true, err
	}
	return r, true, nil
}

// checkAuthBootstrapFlag is a pre-commit hook for cluster create/update operation that makes sure that bootstrap flag is set to true only once for cluster update
func (cl *clusterHooks) checkAuthBootstrapFlag(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Cluster)
	if !ok {
		cl.logger.Errorf("API server hook to check bootstrap flag called for invalid object type [%#v]", i)
		return i, true, errors.New("invalid input type")
	}
	cl.logger.Infof("API server hook called to check bootstrap flag for cluster")

	switch oper {
	case apiintf.CreateOper:
		// always set Bootstrapped flag to false when cluster is created
		r.Status.AuthBootstrapped = false
		return r, true, nil
	case apiintf.UpdateOper:
		cur := &cluster.Cluster{}
		if err := kv.Get(ctx, key, cur); err != nil {
			cl.logger.Errorf("Error getting cluster with key [%s] in API server checkAuthBootstrapFlag pre-commit hook for create/update cluster", key)
			return r, true, err
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
		return i, true, errors.New("invalid input type")
	}
}

// setAuthBootstrapFlag is a pre-commit hook to set bootstrap flag
func (cl *clusterHooks) setAuthBootstrapFlag(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
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
		genID, err := strconv.ParseInt(clusterObj.GenerationID, 10, 64)
		if err != nil {
			cl.logger.ErrorLog("method", "setAuthBootstrapFlag", "msg", "error parsing generation ID", "error", err)
			return clusterObj, err
		}
		clusterObj.GenerationID = fmt.Sprintf("%d", genID+1)
		return clusterObj, nil
	}); err != nil {
		cl.logger.Errorf("Error setting bootstrap flag: %v", err)
		return nil, false, err
	}
	cl.logger.Infof("Cluster bootstrap flag is set and locked down")
	return *cur, false, nil
}

func (cl *clusterHooks) getClusterObject(ctx context.Context, kvs kvstore.Interface, prefix string, in, old, resp interface{}, oper apiintf.APIOperType) (interface{}, error) {
	ic := in.(cluster.Cluster)
	key := ic.MakeKey("cluster")
	cur := cluster.Cluster{}
	if err := kvs.Get(ctx, key, &cur); err != nil {
		cl.logger.Errorf("Error getting cluster with key [%s] in API server getClusterObject response writer hook for create/update cluster", key)
		return nil, err
	}
	if err := cur.ApplyStorageTransformer(ctx, false); err != nil {
		cl.logger.Errorf("error applying storage transformer: %v", err)
		return nil, err
	}
	cur.Spec.Key = ""
	return cur, nil
}

// populateExistingTLSConfig is a pre-commit hook for cluster update operation to populate existing TLS certificate and key. It ignores certs and key passed in the cluster object.
// User will need to use UpdateTLSConfig action on cluster object to update TLS config
func (cl *clusterHooks) populateExistingTLSConfig(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	cl.logger.DebugLog("method", "populateExistingTLSConfig", "msg", "API server hook called to populate TLS config in cluster object")
	r, ok := i.(cluster.Cluster)
	if !ok {
		cl.logger.ErrorLog("method", "populateExistingTLSConfig", "msg", fmt.Sprintf("called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}

	switch oper {
	case apiintf.UpdateOper:
		cur := &cluster.Cluster{}
		if err := kv.Get(ctx, key, cur); err != nil {
			cl.logger.ErrorLog("method", "populateExistingTLSConfig",
				"msg", fmt.Sprintf("error getting cluster with key [%s] in API server pre-commit hook for update cluster", key),
				"error", err)
			return r, true, err
		}
		r.Spec.Certs = cur.Spec.Certs
		// decrypt key as it is stored as secret. Cannot use passed in context because peer id in it is APIGw and transform returns empty key in that case
		if err := cur.ApplyStorageTransformer(context.Background(), false); err != nil {
			cl.logger.ErrorLog("method", "populateExistingTLSConfig", "msg", "error decrypting key field", "error", err)
			return r, true, err
		}
		r.Spec.Key = cur.Spec.Key
		// Add a comparator for CAS
		cl.logger.InfoLog("method", "populateExistingTLSConfig", "msg", fmt.Sprintf("set the comparator version for [%s] as [%s]", key, cur.ResourceVersion))
		txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", cur.ResourceVersion))
		return r, true, nil
	default:
		cl.logger.ErrorLog("method", "populateExistingTLSConfig", "msg", fmt.Sprintf("API server hook to set TLS Config called for invalid API Operation [%s]", oper))
		return i, true, errors.New("invalid input type")
	}
}

// setTLSConfig is a pre-commit hook to set TLS config for API Gateway for UpdateTLSConfig action on cluster
func (cl *clusterHooks) setTLSConfig(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	cl.logger.DebugLog("method", "setTLSConfig", "msg", "API server hook called to set TLS Config")
	req, ok := i.(cluster.UpdateTLSConfigRequest)
	if !ok {
		cl.logger.ErrorLog("method", "setTLSConfig", "msg", fmt.Sprintf("API server hook to update TLS Config called for invalid object type [%#v]", i))
		return nil, false, errors.New("invalid input type")
	}
	cur := &cluster.Cluster{}
	if err := kv.ConsistentUpdate(ctx, key, cur, func(oldObj runtime.Object) (runtime.Object, error) {
		clusterObj, ok := oldObj.(*cluster.Cluster)
		if !ok {
			return oldObj, errors.New("invalid input type")
		}
		// if only cert is being updated use existing key
		// decrypt TLS key as it is stored as secret. Cannot use passed in context because peer id in it is APIGw and transform returns empty key in that case
		if err := clusterObj.ApplyStorageTransformer(context.Background(), false); err != nil {
			cl.logger.ErrorLog("method", "setTLSConfig", "msg", "error decrypting TLS key field", "error", err)
			return clusterObj, err
		}
		tlsKey := clusterObj.Spec.Key
		// if key is also being updated
		if req.Key != "" {
			tlsKey = req.Key
		}
		_, err := tls.X509KeyPair([]byte(req.Certs), []byte(tlsKey))
		if err != nil {
			cl.logger.ErrorLog("method", "setTLSConfig", "msg", "error parsing cert and key", "error", err)
			return oldObj, err
		}
		clusterObj.Spec.Certs = req.Certs
		clusterObj.Spec.Key = tlsKey
		// encrypt TLS key as it is stored as secret
		if err := clusterObj.ApplyStorageTransformer(ctx, true); err != nil {
			cl.logger.ErrorLog("method", "setTLSConfig", "msg", "error encrypting TLS key field", "error", err)
			return clusterObj, err
		}
		genID, err := strconv.ParseInt(clusterObj.GenerationID, 10, 64)
		if err != nil {
			cl.logger.ErrorLog("method", "setTLSConfig", "msg", "error parsing generation ID", "error", err)
			return clusterObj, err
		}
		clusterObj.GenerationID = fmt.Sprintf("%d", genID+1)
		return clusterObj, nil
	}); err != nil {
		cl.logger.ErrorLog("method", "setTLSConfig", "msg", "error updating cert and key", "error", err)
		return nil, false, err
	}
	// decrypt TLS key before returning. Create a copy as cur is pointing to an object in API server cache.
	ret, err := cur.Clone(nil)
	if err != nil {
		cl.logger.ErrorLog("method", "setTLSConfig", "msg", "error creating a copy of cluster obj", "error", err)
		return nil, false, err
	}
	cluster := ret.(*cluster.Cluster)
	cluster.Spec.Key = ""
	cl.logger.InfoLog("method", "setTLSConfig", "msg", "Cluster TLS Config has been updated")
	return *cluster, false, nil
}

func registerClusterHooks(svc apiserver.Service, logger log.Logger) {
	r := clusterHooks{}
	r.logger = logger.WithContext("Service", "ClusterHooks")
	logger.Log("msg", "registering Hooks for cluster apigroup")
	svc.GetCrudService("Host", apiintf.CreateOper).WithPreCommitHook(r.hostPreCommitHook).GetRequestType().WithValidate(r.validateHostConfig)
	svc.GetCrudService("Host", apiintf.UpdateOper).WithPreCommitHook(r.hostPreCommitHook).GetRequestType().WithValidate(r.validateHostConfig)
	svc.GetCrudService("Node", apiintf.CreateOper).GetRequestType().WithValidate(r.validateNodeConfig)
	svc.GetCrudService("Node", apiintf.UpdateOper).GetRequestType().WithValidate(r.validateNodeConfig)
	svc.GetCrudService("Cluster", apiintf.CreateOper).WithPreCommitHook(r.checkAuthBootstrapFlag).GetRequestType().WithValidate(r.validateClusterConfig)
	svc.GetCrudService("Cluster", apiintf.UpdateOper).WithPreCommitHook(r.checkAuthBootstrapFlag).WithPreCommitHook(r.populateExistingTLSConfig).GetRequestType().WithValidate(r.validateClusterConfig)
	svc.GetCrudService("SmartNIC", apiintf.UpdateOper).WithPreCommitHook(r.smartNICPreCommitHook)
	svc.GetCrudService("SmartNIC", apiintf.DeleteOper).WithPreCommitHook(r.smartNICPreCommitHook)
	// hook to set bootstrap flag
	svc.GetMethod("AuthBootstrapComplete").WithPreCommitHook(r.setAuthBootstrapFlag)
	svc.GetMethod("AuthBootstrapComplete").WithResponseWriter(r.getClusterObject)
	// hook to implement update TLS Config action
	svc.GetMethod("UpdateTLSConfig").WithPreCommitHook(r.setTLSConfig)
	svc.GetMethod("UpdateTLSConfig").WithResponseWriter(r.getClusterObject)
	// Only allow default tenant in this release
	svc.GetCrudService("Tenant", apiintf.CreateOper).GetRequestType().WithValidate(r.validateTenant)
	// register hook to create default roles
	svc.GetCrudService("Tenant", apiintf.CreateOper).WithPreCommitHook(r.createDefaultRoles)
	svc.GetCrudService("Tenant", apiintf.CreateOper).WithPreCommitHook(r.createFirewallProfile)
	svc.GetCrudService("Tenant", apiintf.CreateOper).WithPreCommitHook(r.createDefaultVirtualRouter)
	svc.GetCrudService("Tenant", apiintf.CreateOper).WithPreCommitHook(r.createDefaultAlertPolicy)
	svc.GetCrudService("Tenant", apiintf.DeleteOper).WithPreCommitHook(r.deleteDefaultRoles)
	svc.GetCrudService("Tenant", apiintf.DeleteOper).WithPreCommitHook(r.deleteFirewallProfile)
	svc.GetCrudService("Tenant", apiintf.DeleteOper).WithPreCommitHook(r.deleteDefaultVirtualRouter)
	svc.GetCrudService("Tenant", apiintf.DeleteOper).WithPreCommitHook(r.deleteDefaultTelemetryPolicies)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("cluster.ClusterV1", registerClusterHooks)
}
