package impl

import (
	"context"
	"crypto/tls"
	"errors"
	"fmt"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/venice/utils/version"

	"github.com/pensando/sw/api/generated/rollout"

	"github.com/pensando/sw/venice/utils/featureflags"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"
	"google.golang.org/grpc/codes"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/api/generated/security"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/login"
	apiutils "github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	objclient "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
)

type clusterHooks struct {
	logger log.Logger

	// only one restore operation is allowed at any point
	restoreInProgressMu sync.Mutex
	restoreInProgress   bool
}

const (
	relAMajorVersion = 1
	relAMinorVersion = 3
)

// ClusterHooksObjStoreClient is used for testing and overrides the objectstore client used by the hooks.
var ClusterHooksObjStoreClient objclient.Client

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
	fwlogpol := monitoring.FwlogPolicy{ObjectMeta: api.ObjectMeta{Name: r.Name, Tenant: r.Name}}
	fwlogkey := fwlogpol.MakeKey("monitoring")
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
	vrf.Namespace = globals.DefaultNamespace
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
	vrf.Namespace = globals.DefaultNamespace
	vrfk := vrf.MakeKey(string(apiclient.GroupNetwork))
	err := txn.Delete(vrfk)
	if err != nil {
		return r, true, errors.New("adding delete operation to transaction failed")
	}
	return r, true, nil
}

// createDefaultRoutingTable is a pre-commit hook to creates default RouteTable when a tenant is created
func (cl *clusterHooks) createDefaultRouteTable(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.ErrorLog("method", "createDefaultVirtualRouter", "msg", fmt.Sprintf("API server hook to create default VRF called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	rt := &network.RouteTable{}
	rt.Defaults("all")
	apiSrv := apisrvpkg.MustGetAPIServer()
	rt.APIVersion = apiSrv.GetVersion()
	rt.SelfLink = rt.MakeURI("configs", rt.APIVersion, string(apiclient.GroupNetwork))
	rt.Name = "default.default"
	rt.Tenant = r.Name
	rt.Namespace = globals.DefaultNamespace
	rt.GenerationID = "1"
	rt.UUID = uuid.NewV4().String()
	ts, err := types.TimestampProto(time.Now())
	if err != nil {
		return i, true, err
	}
	rt.CreationTime, rt.ModTime = api.Timestamp{Timestamp: *ts}, api.Timestamp{Timestamp: *ts}
	rtk := rt.MakeKey(string(apiclient.GroupNetwork))
	err = txn.Create(rtk, rt)
	if err != nil {
		return r, true, errors.New("adding create operation to transaction failed")
	}
	return r, true, nil
}

// createDefaultDSCProfile is a pre-commit hook to creates default RouteTable when a tenant is created
func (cl *clusterHooks) createDefaultDSCProfile(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.ErrorLog("method", "createDefaultDSCProfile", "msg", fmt.Sprintf("API server hook to create default dscProfile called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	if r.GetName() != globals.DefaultTenant {
		return r, true, nil
	}

	rt := &cluster.DSCProfile{}
	rt.Defaults("all")
	apiSrv := apisrvpkg.MustGetAPIServer()
	rt.APIVersion = apiSrv.GetVersion()
	rt.SelfLink = rt.MakeURI("configs", rt.APIVersion, string(apiclient.GroupCluster))
	rt.Name = globals.DefaultDSCProfile
	rt.Tenant = ""
	rt.Namespace = ""
	rt.GenerationID = "1"
	rt.UUID = uuid.NewV4().String()
	ts, err := types.TimestampProto(time.Now())
	if err != nil {
		return i, true, err
	}
	rt.CreationTime, rt.ModTime = api.Timestamp{Timestamp: *ts}, api.Timestamp{Timestamp: *ts}
	rtk := rt.MakeKey(string(apiclient.GroupCluster))
	err = txn.Create(rtk, rt)
	if err != nil {
		return r, true, errors.New("adding create operation to transaction failed")
	}
	return r, true, nil
}

// deleteDefaultRoutingTable is a pre-commit hook to delete default RouteTable when a tenant is deleted
func (cl *clusterHooks) deleteDefaultRouteTable(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.ErrorLog("method", "deleteDefaultVirtualRouter", "msg", fmt.Sprintf("API server hook to delete default firewall profile called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	rt := &network.RouteTable{}
	rt.Defaults("all")
	apiSrv := apisrvpkg.MustGetAPIServer()
	rt.APIVersion = apiSrv.GetVersion()
	rt.SelfLink = rt.MakeURI("configs", rt.APIVersion, string(apiclient.GroupNetwork))
	rt.Name = "default.default"
	rt.Tenant = r.Name
	rt.Namespace = globals.DefaultNamespace
	rtk := rt.MakeKey(string(apiclient.GroupNetwork))
	err := txn.Delete(rtk)
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
	if !(flags.AllowMultiTenant || featureflags.IsMultiTenantEnabled()) && r.Name != globals.DefaultTenant {
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

func (cl *clusterHooks) deleteDefaultAlertPolicy(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(cluster.Tenant)
	if !ok {
		cl.logger.ErrorLog("method", "deleteDefaultAlertPolicy", "msg", fmt.Sprintf("API server hook to delete default alert policy called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	cl.logger.DebugLog("method", "deleteDefaultAlertPolicy", "msg", fmt.Sprintf("API server hook called to delete default alert policy for tenant [%v]", r.Name))

	alertPolicy := &monitoring.AlertPolicy{}
	alertPolicy.Defaults("all")
	alertPolicy.Name = "default-event-based-alerts"
	alertPolicy.Tenant = r.GetName()
	alertPolicy.Namespace = globals.DefaultNamespace
	if err := txn.Delete(alertPolicy.MakeKey(string(apiclient.GroupMonitoring))); err != nil {
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

func (cl *clusterHooks) getRestoreObject(ctx context.Context, kvs kvstore.Interface, prefix string, in, old, resp interface{}, oper apiintf.APIOperType) (interface{}, error) {
	ic := cluster.SnapshotRestore{}
	key := ic.MakeKey("cluster")
	if err := kvs.Get(ctx, key, &ic); err != nil {
		cl.logger.Errorf("Error getting Snapshot with key [%s] in API server getSnapshotObject response writer hook for create/update Snapshot", key)
		return nil, err
	}

	return ic, nil
}

func (cl *clusterHooks) getSnapshotObject(ctx context.Context, kvs kvstore.Interface, prefix string, in, old, resp interface{}, oper apiintf.APIOperType) (interface{}, error) {
	ic := cluster.ConfigurationSnapshot{}
	key := ic.MakeKey("cluster")
	cur := cluster.ConfigurationSnapshot{}
	if err := kvs.Get(ctx, key, &cur); err != nil {
		cl.logger.Errorf("Error getting Snapshot with key [%s] in API server getSnapshotObject response writer hook for create/update Snapshot", key)
		return nil, err
	}
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

func (cl *clusterHooks) getObjstoreClient() (objclient.Client, error) {
	if ClusterHooksObjStoreClient != nil {
		return ClusterHooksObjStoreClient, nil
	}
	apisrv := apisrvpkg.MustGetAPIServer()
	resolvers := apisrv.GetResolvers()
	rslvrs := resolver.New(&resolver.Config{Name: globals.APIServer, Servers: resolvers})

	tlsp, err := rpckit.GetDefaultTLSProvider(globals.APIServer)
	if err != nil {
		return nil, fmt.Errorf("[%v]error getting tls provider", err)
	}

	tlsc, err := tlsp.GetClientTLSConfig(globals.APIServer)
	if err != nil {
		return nil, fmt.Errorf("[%v]error getting tls client", err)
	}
	tlsc.ServerName = globals.Vos

	oclnt, err := objclient.NewClient(globals.DefaultTenant, objstore.Buckets_snapshots.String(), rslvrs, objclient.WithTLSConfig(tlsc))
	if err != nil {
		return nil, fmt.Errorf("[%v]creating objstore client", err)
	}
	return oclnt, nil
}

func (cl *clusterHooks) writeSnapshot(ctx context.Context, name string, client objclient.Client, meta map[string]string) (uint64, error) {
	apisrvcache := apisrvpkg.GetAPIServerCache()

	// Get object store config.
	rev := apisrvcache.StartSnapshot()
	if rev == apiintf.InvalidSnapShotHandle {
		return 0, errors.New("could not start a config snapshot")
	}

	rdr, err := apisrvcache.SnapshotReader(rev, false, nil)
	if err != nil {
		return 0, fmt.Errorf("[%v] creating snapshot reade", err)
	}
	defer rdr.Close()

	meta["Creation-Time"] = fmt.Sprintf("%s", time.Now().Format(time.RFC3339Nano))

	// XXX-TODO(sanjayt): Add metadata
	written, err := client.PutObject(ctx, name, rdr, meta)
	log.Infof("wrote [%d] bytes with error (%v)", written, err)
	return rev, err
}

// performRestoreNow restores the configuration from a snapshot
// Sequence is
//  1. Validate there is no other restore in progress
//      a. check in memorey mutex
//      b. check object store, in case api server has restarted
//  2. In the overlay
//      a. Delete all existing objects [excluding exceptions like cluster, module and networkintf objects]
//      b. read and apply objects in snapshot to overlay
//  3. Commit the overlay
//
//  Error handling:
//   - failures at or before 2 doesnt need much handling since the kvstore has not been touched
//   - failure at 3 : try to recover from the backup snapshot (in memory cache)
func (cl *clusterHooks) performRestoreNow(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	// Check if Rollout is in progress. Reject if so.
	verObj := cluster.Version{}
	vkey := verObj.MakeKey(string(apiclient.GroupCluster))
	err := kvs.Get(ctx, vkey, &verObj)
	if err == nil && verObj.Status.RolloutBuildVersion != "" {
		return i, false, errors.New("rollout in progress, restore operation not allowed")
	}
	cl.restoreInProgressMu.Lock()
	if cl.restoreInProgress {
		cl.restoreInProgressMu.Unlock()
		log.Errorf("[config-restore] could not lock restoreMutex")
		return i, false, errors.New("restore operation in progress, try again later")
	}
	cl.restoreInProgress = true
	cl.restoreInProgressMu.Unlock()

	defer func() {
		cl.restoreInProgressMu.Lock()
		cl.restoreInProgress = false
		cl.restoreInProgressMu.Unlock()
	}()

	apiSrv := apisrvpkg.MustGetAPIServer()

	fl := apiSrv.RuntimeFlags()
	flags := fl.InternalParams
	var sleepOnRestore int
	var failSetObj, failPrep, failWrite, failCommit bool
	if v, ok := flags["sleep-on-restore"]; ok {
		in, err := strconv.ParseInt(v, 10, 32)
		if err == nil {
			sleepOnRestore = int(in)
		}
	}
	if v, ok := flags["fail-set-obj"]; ok {
		in, err := strconv.ParseBool(v)
		if err == nil {
			failSetObj = in
		}
	}
	if v, ok := flags["fail-prep"]; ok {
		in, err := strconv.ParseBool(v)
		if err == nil {
			failPrep = in
		}
	}
	if v, ok := flags["fail-write"]; ok {
		in, err := strconv.ParseBool(v)
		if err == nil {
			failWrite = in
		}
	}
	if v, ok := flags["fail-commit"]; ok {
		in, err := strconv.ParseBool(v)
		if err == nil {
			failCommit = in
		}
	}

	// Validate Object
	obj := i.(cluster.SnapshotRestore)
	rstat := cluster.SnapshotRestore{}
	rkey := rstat.MakeKey(string(apiclient.GroupCluster))

	log.Infof("[config-restore] starting restore operation to snapshot [%v]", obj.Spec.SnapshotPath)

	pctx := apiutils.SetVar(ctx, apiutils.CtxKeyAPISrvLargeBuffer, true)
	pctx = apiutils.SetVar(ctx, apiutils.CtxKeyAPISrvInitRestore, true)
	dctx := apiutils.SetVar(context.Background(), apiutils.CtxKeyPersistDirectKV, true)
	var backupRev uint64
	found := false
	backupName := ""

	// Get Object store
	oclnt, err := cl.getObjstoreClient()
	if err != nil {
		return i, false, err
	}
	rdr, err := oclnt.GetObject(ctx, obj.Spec.SnapshotPath)
	if err != nil {
		log.Errorf("Failed to retrieve sapshot, configuration unchanged (%s)", err)
		recorder.Event(eventtypes.CONFIG_RESTORE_ABORTED, fmt.Sprintf("failed to retrieve snapshot [%v], Configuration unchanged", obj.Spec.SnapshotPath), &obj)
		return rstat, false, &api.Status{
			Result:      api.StatusResult{Str: "failed to retrieve snapshot, Configuration unchanged"},
			Message:     []string{fmt.Sprintf("%v", err)},
			Code:        int32(codes.FailedPrecondition),
			IsTemporary: false,
		}
	}
	err = kvs.Get(ctx, rkey, &rstat)
	if err == nil {
		found = true
		if rstat.Status.Status == cluster.SnapshotRestoreStatus_Active.String() {
			log.Errorf("[config-restore] recovering from restore operation from previous lifetime.")
			backupName = rstat.Status.BackupSnapshotPath
		}
		rstat.Spec = obj.Spec
	}

	log.Infof("[config-restore] snapshot [%v] is valid, proceeding to restore", obj.Spec.SnapshotPath)

	// Lock the apiserver to stop processing any more API calls
	ctrl := apiserver.Controls{MaintMode: true, MaintReason: "Configuration restore operation"}
	apiSrv.SetRuntimeControls(ctrl)
	defer func() {
		ctrl := apiserver.Controls{MaintMode: false, MaintReason: ""}
		apiSrv.SetRuntimeControls(ctrl)
	}()

	if sleepOnRestore > 0 {
		time.Sleep(time.Duration(sleepOnRestore) * time.Second)
	}

	apisrvcache := apisrvpkg.GetAPIServerCache()
	kinds := runtime.GetDefaultScheme().Kinds()

	delFromCache := func(ictx context.Context, kvi kvstore.Interface) error {
		for g, v := range kinds {
			for _, k := range v {
				if !apiutils.IsSavedKind(k) {
					continue
				}
				stat, err := apisrvcache.StatKind(g, k)
				if err != nil {
					log.Errorf("[config-restore] failed to stat objects for [%v/%v](%v)", g, k, err)
					return fmt.Errorf("failed to stat objects for [%v/%v](%v)", g, k, err)
				}
				log.Debugf("[config-restore] deleting [%v/%v]", g, k)
				for i := range stat {
					if stat[i].Valid {
						kvi.Delete(ictx, stat[i].Key, nil)
					}
				}
			}
		}
		return nil
	}

	rollback := func(rctx context.Context) *api.Status {
		log.Infof("[config-restore] rollback called - getting to clean state")
		var e1 error
		if ov, ok := kvs.(apiintf.OverlayInterface); ok {
			e1 = ov.ClearBuffer(rctx, nil)
		}
		if e1 == nil {
			delFromCache(rctx, kvs)
		}
		log.Infof("[config-restore] attempting config rollback")
		if e1 == nil {
			e1 = apisrvcache.Rollback(rctx, backupRev, kvs)
			if e1 == nil {
				_, e1 = txn.Commit(rctx)
			}
		}
		if e1 != nil {
			log.Errorf("[config-restore] could not rollback to backup revision (%s)", e1)
			recorder.Event(eventtypes.CONFIG_RESTORE_FAILED, fmt.Sprintf("configuration restore operation for [%v] failed. Configuration could not be rolled back", obj.Spec.SnapshotPath), &obj)
			return &api.Status{
				Result:      api.StatusResult{Str: "configuration restore operation failed. Configuration could not be rolled back"},
				Message:     []string{fmt.Sprintf("%v", e1)},
				Code:        int32(codes.Aborted),
				IsTemporary: false,
			}
		}
		log.Infof("[config-restore] rollback to rev [%v] succeeded", backupRev)
		return nil

	}

	err = delFromCache(pctx, kvs)
	if err != nil || failPrep {
		log.Errorf("[config-restore] failed Prep for restore (%v)", err)
		recorder.Event(eventtypes.CONFIG_RESTORE_ABORTED, fmt.Sprintf("failed preparation for restore operation for [%v]. Configuration unchanged", obj.Spec.SnapshotPath), &obj)
		return rstat, false, &api.Status{
			Result:      api.StatusResult{Str: "failed preparation for restore operation. Configuration unchanged"},
			Message:     []string{fmt.Sprintf("%v", err)},
			Code:        int32(codes.FailedPrecondition),
			IsTemporary: false,
		}
	}

	log.Infof("[config-restore] [%v] delete from overlay succeeded, starting write to overlay", obj.Spec.SnapshotPath)
	writer := apisrvcache.SnapshotWriter(rdr)
	err = writer.Write(pctx, kvs)
	if err != nil || failWrite {
		log.Errorf("[config-restpore] applying snapshot failed (%s)", err)
		recorder.Event(eventtypes.CONFIG_RESTORE_ABORTED, fmt.Sprintf("configuration restore operation for [%v] failed . Configuration unchanged", obj.Spec.SnapshotPath), &obj)
		retErr := api.Status{
			Result:      api.StatusResult{Str: "configuration restore operation failed. Configuration unchanged"},
			Message:     []string{err.Error()},
			Code:        int32(codes.FailedPrecondition),
			IsTemporary: false,
		}
		return rstat, false, &retErr
	}

	// Prepare to commit the restore buffer
	if backupName == "" {
		backupName = "backup" + time.Now().Format(time.RFC3339)
		backupName = strings.Replace(backupName, " ", "_", -1)
		backupName = strings.Replace(backupName, ":", "-", -1)
		backupName = strings.Replace(backupName, "+", "", -1)
		log.Infof("[config-restore] creating backup snapshot at %s", backupName)
		meta := map[string]string{
			"RequestName": rstat.Name,
		}
		backupRev, err = cl.writeSnapshot(ctx, backupName, oclnt, meta)
		if err != nil || failSetObj {
			log.Errorf("[config-restore] failed to create backup snapshot (%s)", err)
			recorder.Event(eventtypes.CONFIG_RESTORE_ABORTED, fmt.Sprintf("failed to create backup snapshot for [%v], Configuration unchanged", obj.Spec.SnapshotPath), &obj)
			return rstat, false, &api.Status{
				Result:      api.StatusResult{Str: "failed to create backup snapshot, Configuration unchanged"},
				Message:     []string{fmt.Sprintf("%v", err)},
				Code:        int32(codes.FailedPrecondition),
				IsTemporary: false,
			}
		}
	}

	log.Infof("[config-restore] Committing restore overlay [%v]", obj.Spec.SnapshotPath)
	rstat.TypeMeta = obj.TypeMeta
	rstat.Spec = obj.Spec
	rstat.Status = cluster.SnapshotRestoreStatus{}
	rstat.Status.Status = cluster.SnapshotRestoreStatus_Active.String()
	rstat.Status.BackupSnapshotPath = backupName
	ts, _ := types.TimestampProto(time.Now())
	rstat.Status.StartTime = &api.Timestamp{Timestamp: *ts}
	rstat.ObjectMeta.ModTime = api.Timestamp{Timestamp: *ts}
	rstat.SelfLink = rstat.MakeURI("configs", "v1", string(apiclient.GroupCluster))
	rstat.Status.Status = cluster.SnapshotRestoreStatus_Active.String()
	if found {
		err = kvs.Update(dctx, rkey, &rstat)
		if err != nil {
			log.Errorf("[config-restore] failed to update restore object before applying snapshot(%s)", err)
			recorder.Event(eventtypes.CONFIG_RESTORE_ABORTED, fmt.Sprintf(" failed to update restore object before applying snapshot [%v], Configuration unchanged", obj.Spec.SnapshotPath), &obj)
			return rstat, false, &api.Status{
				Result:      api.StatusResult{Str: "configuration restore operation failed. Configuration unchanged"},
				Message:     []string{"failed to write restore object before applying snapshot"},
				Code:        int32(codes.FailedPrecondition),
				IsTemporary: false,
			}
		}
	} else {
		rstat.ObjectMeta.CreationTime = api.Timestamp{Timestamp: *ts}
		rstat.UUID = uuid.NewV4().String()
		err = kvs.Create(dctx, rkey, &rstat)
		if err != nil {
			log.Errorf("[config-restore] failed to create restore object before applying snapshot(%s)", err)
			recorder.Event(eventtypes.CONFIG_RESTORE_ABORTED, fmt.Sprintf("failed to create restore object before applying snapshot [%v], Configuration unchanged", obj.Spec.SnapshotPath), &obj)
			return rstat, false, &api.Status{
				Result:      api.StatusResult{Str: "configuration restore operation failed. Configuration unchanged"},
				Message:     []string{"failed to write restore object before applying snapshot"},
				Code:        int32(codes.FailedPrecondition),
				IsTemporary: false,
			}
		}
	}

	// Point of no return. Will need rollback.
	// Dont let a cancel on the call context interrupt this go routing after this point.
	npctx := apiutils.SetVar(context.Background(), apiutils.CtxKeyAPISrvLargeBuffer, true)
	npctx = apiutils.SetVar(npctx, apiutils.CtxKeyAPISrvInitRestore, true)
	ndctx := apiutils.SetVar(context.Background(), apiutils.CtxKeyPersistDirectKV, true)
	// Commit the buffer so any errors can be captured here.
	//  The txn is a surrogate for the Ov, commit
	_, err = txn.Commit(npctx)
	if err != nil || failCommit {
		log.Errorf("[config-restore] commit failed (%s), attempting rollback", err)
		status := rollback(npctx)
		if status != nil {
			return rstat, false, status
		}
		recorder.Event(eventtypes.CONFIG_RESTORE_ABORTED, fmt.Sprintf("configuration restore operation for [%v] failed. Configuration unchanged", obj.Spec.SnapshotPath), &obj)
		return rstat, false, &api.Status{
			Result:      api.StatusResult{Str: "configuration restore operation commit failed. Configuration was rolled back"},
			Message:     []string{fmt.Sprintf("%v", err)},
			Code:        int32(codes.FailedPrecondition),
			IsTemporary: false,
		}
	}

	rstat.Status.Status = cluster.SnapshotRestoreStatus_Completed.String()
	ts, _ = types.TimestampProto(time.Now())
	rstat.Status.EndTime = &api.Timestamp{Timestamp: *ts}
	err = kvs.Update(ndctx, rkey, &rstat)
	if err != nil {
		return rstat, false, fmt.Errorf("could not update the Restore Object")
	}

	log.Infof("[config-resore] deleting object [%s]", backupName)
	err = oclnt.RemoveObject(backupName)
	if err != nil {
		log.Errorf("[config-restore] failed to delete backup from objectore [%s]", err)
		return rstat, true, &api.Status{
			Result:      api.StatusResult{Str: "configuration restore operation succeeded, but failed to cleanup temporary backup snapshot"},
			Message:     []string{err.Error()},
			Code:        int32(codes.Internal),
			IsTemporary: false,
		}
	}
	log.Infof("[config-restore] completed operation, snapshot [%v]", obj.Spec.SnapshotPath)

	if ov, ok := kvs.(apiintf.OverlayInterface); ok {
		log.Infof("Clearing buffer before returning")
		ov.ClearBuffer(ctx, nil)
	}
	log.Infof("Restore operation completed successfully for [%s][%v]", obj.Name, obj.Spec.SnapshotPath)
	recorder.Event(eventtypes.CONFIG_RESTORED, fmt.Sprintf("configuration was restored to snapshot [%v]", obj.Spec.SnapshotPath), &obj)
	return rstat, false, nil
}

func (cl *clusterHooks) performSnapshotNow(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	req := i.(cluster.ConfigurationSnapshotRequest)
	// Fetch the snapshot config from KVstoreapi/utils/apiutils.go
	scfg := cluster.ConfigurationSnapshot{}
	skey := scfg.MakeKey(string(apiclient.GroupCluster))

	err := kvs.Get(ctx, skey, &scfg)
	if err != nil {
		return scfg, false, fmt.Errorf("could not get Sanpshot config (%s)", err)
	}

	if scfg.Spec.Destination.Type != cluster.SnapshotDestinationType_ObjectStore.String() {
		return scfg, false, fmt.Errorf("only object store destination is supported")
	}

	oclnt, err := cl.getObjstoreClient()
	if err != nil {
		return i, false, err
	}

	meta := map[string]string{
		"RequestName": req.Name,
	}
	name := "snapshot-" + strings.Replace(strings.Replace(time.Now().Format(time.UnixDate), ":", "-", -1), " ", "_", -1)
	if req.Name != "" {
		name = "snapshot-" + req.Name + "-" + strings.Replace(strings.Replace(time.Now().Format(time.UnixDate), ":", "-", -1), " ", "_", -1)
	}

	_, err = cl.writeSnapshot(ctx, name, oclnt, meta)

	scfg.Status.LastSnapshot = &cluster.ConfigurationSnapshotStatus_ConfigSaveStatus{
		DestType: cluster.SnapshotDestinationType_ObjectStore.String(),
		URI:      fmt.Sprintf("/objstore/v1/downloads/snapshots/%s", name),
	}
	err = txn.Update(skey, &scfg)
	return scfg, false, err
}

func (cl *clusterHooks) validateFFBootstrap(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
	ff := i.(cluster.License)
	_, errs := featureflags.Validate(ff.Spec.Features)
	return errs
}

// checkFFBootstrap checks if the FeatureFlags can be updated
func (cl *clusterHooks) checkFFBootstrap(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	ff := i.(cluster.License)
	ffs, errs := featureflags.Validate(ff.Spec.Features)
	if len(errs) != 0 {
		str := ""
		for _, err := range errs {
			str = str + "[" + fmt.Sprintf("%s", err) + "]"
		}
		return i, false, fmt.Errorf("failed applying feature flags %s", str)
	}
	ff.Status = ffs
	return ff, true, nil
}

func (cl *clusterHooks) nodePreCommitHook(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	//create default firewall profile under the following conditions
	// 1. upgrade from A Release to B Release (1.3 to 1.4 & up)
	// 2. apiserver restart during upgrade

	//Node precommit is never failed for dsc profile. We log error and continue
	cl.logger.Infof("(nodePreCommitHook) Get ClusterVersion Object")
	verObj := cluster.Version{}
	vkey := verObj.MakeKey(string(apiclient.GroupCluster))
	err := kvs.Get(ctx, vkey, &verObj)
	if err == nil && verObj.Status.RolloutBuildVersion == "" {
		cl.logger.Infof("(nodePreCommitHook) Rollout not in progress")
		return i, true, nil
	}
	cl.logger.Infof("(nodePreCommitHook) Get RolloutAction Object")
	rolloutActionObj := &rollout.RolloutAction{}
	rolloutActionObjKey := rolloutActionObj.MakeKey(string(apiclient.GroupRollout))

	if err = kvs.Get(ctx, rolloutActionObjKey, rolloutActionObj); err != nil {
		cl.logger.Infof("(nodePreCommitHook) RolloutAction object not present")
		return i, true, nil
	}

	//Assumption A release starts with 1.3 && B Release starts with 1.4 & up
	rolloutMajorVersion := version.GetMajorVersion(rolloutActionObj.Spec.Version)
	rolloutMinorVersion := version.GetMinorVersion(rolloutActionObj.Spec.Version)
	if rolloutMajorVersion == 0 || rolloutMinorVersion == 0 {
		cl.logger.Errorf("(nodePreCommitHook) Invalid Version")
		return i, true, nil
	}
	//upgrade from 1.3 to 1.4+ but not to 2.x
	if strings.HasPrefix(verObj.Status.BuildVersion, "1.3") && rolloutMajorVersion == relAMajorVersion && rolloutMinorVersion > relAMinorVersion {
		insertionFWProfile := &cluster.DSCProfile{}
		insertionFWProfile.Defaults("all")
		insertionFWProfile.Spec.FwdMode = cluster.DSCProfileSpec_INSERTION.String()
		insertionFWProfile.Spec.FlowPolicyMode = cluster.DSCProfileSpec_ENFORCED.String()
		apiSrv := apisrvpkg.MustGetAPIServer()
		insertionFWProfile.APIVersion = apiSrv.GetVersion()
		insertionFWProfile.SelfLink = insertionFWProfile.MakeURI("configs", insertionFWProfile.APIVersion, string(apiclient.GroupCluster))
		insertionFWProfile.Name = "InsertionFWProfile"
		insertionFWProfile.Tenant = ""
		insertionFWProfile.Namespace = ""
		insertionFWProfile.GenerationID = "1"
		insertionFWProfile.UUID = uuid.NewV4().String()
		ts, _ := types.TimestampProto(time.Now())

		insertionFWProfile.CreationTime, insertionFWProfile.ModTime = api.Timestamp{Timestamp: *ts}, api.Timestamp{Timestamp: *ts}
		insertionFWProfileKey := insertionFWProfile.MakeKey(string(apiclient.GroupCluster))
		intoFWProfile := cluster.DSCProfile{}
		err = kvs.Get(ctx, insertionFWProfileKey, &intoFWProfile)
		if err != nil {
			cl.logger.Infof("(nodePreCommitHook) InsertionFWProfile not found (%+v). Creating now", err)
			err = txn.Create(insertionFWProfileKey, insertionFWProfile)
			if err != nil {
				cl.logger.Errorf("(nodePreCommitHook)InsertionFWProfile Creation Error %+v", err)
				return i, true, nil
			}
		}

		// Create a Default Profile as well
		dscDefaultProfile := &cluster.DSCProfile{}
		dscDefaultProfile.Defaults("all")
		dscDefaultProfile.APIVersion = apiSrv.GetVersion()
		dscDefaultProfile.SelfLink = dscDefaultProfile.MakeURI("configs", dscDefaultProfile.APIVersion, string(apiclient.GroupCluster))
		dscDefaultProfile.Name = globals.DefaultDSCProfile
		dscDefaultProfile.Tenant = ""
		dscDefaultProfile.Namespace = ""
		dscDefaultProfile.GenerationID = "1"
		dscDefaultProfile.UUID = uuid.NewV4().String()
		timestamp, _ := types.TimestampProto(time.Now())

		dscDefaultProfile.CreationTime, dscDefaultProfile.ModTime = api.Timestamp{Timestamp: *timestamp}, api.Timestamp{Timestamp: *timestamp}
		dscDefaultProfileKey := dscDefaultProfile.MakeKey(string(apiclient.GroupCluster))
		intoDscProfile := cluster.DSCProfile{}
		err = kvs.Get(ctx, dscDefaultProfileKey, &intoDscProfile)
		if err != nil {
			cl.logger.Infof("(nodePreCommitHook) dscDefaultProfile not found (%+v). Creating now", err)
			err = txn.Create(dscDefaultProfileKey, dscDefaultProfile)
			if err != nil {
				cl.logger.Errorf("(nodePreCommitHook)dscDefaultProfile Creation Error %+v", err)
				return i, true, nil
			}
		}

		into := cluster.DistributedServiceCardList{}
		into.Kind = "DistributedServiceCardList"
		r := cluster.DistributedServiceCard{}
		keyDSC := r.MakeKey(string(apiclient.GroupCluster))

		ctx = apiutils.SetVar(ctx, "ObjKind", "cluster.DistributedServiceCard")
		err = kvs.List(ctx, keyDSC, &into)
		if err != nil {
			cl.logger.ErrorLog("msg", "(nodePreCommitHook) DistributedServiceCardList failed", "key", key, "err", err)
			return nil, true, nil
		}
		cl.logger.Infof("(nodePreCommitHook) DSC List is %+v", into.GetItems())
		for _, s := range into.GetItems() {
			if s.Spec.DSCProfile == "" {
				//Get and Set firewall profile
				s.Spec.DSCProfile = insertionFWProfile.Name
				dscKey := s.MakeKey(string(apiclient.GroupCluster))
				err = kvs.Update(ctx, dscKey, s)
				if err != nil {
					cl.logger.Errorf("(nodePreCommitHook) DSCProfile update to DSCObject %s failed. Error (+%v)", s.Name, err)
				}
			}
		}
	}
	return i, true, nil
}

func (cl *clusterHooks) applyFeatureFlags(ctx context.Context, oper apiintf.APIOperType, i interface{}, dryRun bool) {
	ff := i.(cluster.License)
	featureflags.Update(ff.Spec.Features)
}

func (cl *clusterHooks) restoreFeatureFlags(kvs kvstore.Interface, logger log.Logger) {
	ff := cluster.License{}
	key := ff.MakeKey(string(apiclient.GroupCluster))
	err := kvs.Get(context.TODO(), key, &ff)
	if err == nil {
		featureflags.Update(ff.Spec.Features)
	}
}

func registerClusterHooks(svc apiserver.Service, logger log.Logger) {
	r := clusterHooks{}
	apisrv := apisrvpkg.MustGetAPIServer()
	r.logger = logger.WithContext("Service", "ClusterHooks")
	logger.Log("msg", "registering Hooks for cluster apigroup")
	svc.GetCrudService("Host", apiintf.CreateOper).WithPreCommitHook(r.hostPreCommitHook).GetRequestType().WithValidate(r.validateHostConfig)
	svc.GetCrudService("Host", apiintf.UpdateOper).WithPreCommitHook(r.hostPreCommitHook).GetRequestType().WithValidate(r.validateHostConfig)
	// For hosts created by orchhub
	svc.GetCrudService("Host", apiintf.UpdateOper).WithPreCommitHook(createOrchCheckHook("Host"))
	svc.GetCrudService("Host", apiintf.DeleteOper).WithPreCommitHook(createOrchCheckHook("Host"))

	svc.GetCrudService("Node", apiintf.CreateOper).GetRequestType().WithValidate(r.validateNodeConfig)
	svc.GetCrudService("Node", apiintf.UpdateOper).GetRequestType().WithValidate(r.validateNodeConfig)
	svc.GetCrudService("Node", apiintf.UpdateOper).WithPreCommitHook(r.nodePreCommitHook)
	svc.GetCrudService("Node", apiintf.CreateOper).WithPreCommitHook(r.nodePreCommitHook)
	svc.GetCrudService("Cluster", apiintf.CreateOper).WithPreCommitHook(r.checkAuthBootstrapFlag).GetRequestType().WithValidate(r.validateClusterConfig)
	svc.GetCrudService("Cluster", apiintf.UpdateOper).WithPreCommitHook(r.checkAuthBootstrapFlag).WithPreCommitHook(r.populateExistingTLSConfig).GetRequestType().WithValidate(r.validateClusterConfig)
	svc.GetCrudService("DistributedServiceCard", apiintf.CreateOper).WithPreCommitHook(r.smartNICPreCommitHook)
	svc.GetCrudService("DistributedServiceCard", apiintf.UpdateOper).WithPreCommitHook(r.smartNICPreCommitHook)
	svc.GetCrudService("DistributedServiceCard", apiintf.DeleteOper).WithPreCommitHook(r.smartNICPreCommitHook)

	svc.GetCrudService("DSCProfile", apiintf.CreateOper).WithPreCommitHook(r.DSCProfilePreCommitHook)
	svc.GetCrudService("DSCProfile", apiintf.UpdateOper).WithPreCommitHook(r.DSCProfilePreCommitHook)
	svc.GetCrudService("DSCProfile", apiintf.DeleteOper).WithPreCommitHook(r.DSCProfilePreCommitHook)
	// hook to set bootstrap flag
	svc.GetMethod("AuthBootstrapComplete").WithPreCommitHook(r.setAuthBootstrapFlag)
	svc.GetMethod("AuthBootstrapComplete").WithResponseWriter(r.getClusterObject)
	// hook to implement update TLS Config action
	svc.GetMethod("UpdateTLSConfig").WithPreCommitHook(r.setTLSConfig)
	svc.GetMethod("UpdateTLSConfig").WithResponseWriter(r.getClusterObject)
	svc.GetMethod("Save").WithPreCommitHook(r.performSnapshotNow).WithResponseWriter(r.getSnapshotObject)
	svc.GetMethod("Restore").WithPreCommitHook(r.performRestoreNow).WithResponseWriter(r.getRestoreObject)
	// Only allow default tenant in this release
	svc.GetCrudService("Tenant", apiintf.CreateOper).GetRequestType().WithValidate(r.validateTenant)
	// register hook to create default roles
	svc.GetCrudService("Tenant", apiintf.CreateOper).WithPreCommitHook(r.createDefaultRoles)
	svc.GetCrudService("Tenant", apiintf.CreateOper).WithPreCommitHook(r.createFirewallProfile)
	svc.GetCrudService("Tenant", apiintf.CreateOper).WithPreCommitHook(r.createDefaultVirtualRouter)
	svc.GetCrudService("Tenant", apiintf.CreateOper).WithPreCommitHook(r.createDefaultRouteTable)
	svc.GetCrudService("Tenant", apiintf.CreateOper).WithPreCommitHook(r.createDefaultAlertPolicy)
	svc.GetCrudService("Tenant", apiintf.CreateOper).WithPreCommitHook(r.createDefaultDSCProfile)
	svc.GetCrudService("Tenant", apiintf.DeleteOper).WithPreCommitHook(r.deleteDefaultRoles)
	svc.GetCrudService("Tenant", apiintf.DeleteOper).WithPreCommitHook(r.deleteFirewallProfile)
	svc.GetCrudService("Tenant", apiintf.DeleteOper).WithPreCommitHook(r.deleteDefaultVirtualRouter)
	svc.GetCrudService("Tenant", apiintf.DeleteOper).WithPreCommitHook(r.deleteDefaultRouteTable)
	svc.GetCrudService("Tenant", apiintf.DeleteOper).WithPreCommitHook(r.deleteDefaultAlertPolicy)
	svc.GetCrudService("Tenant", apiintf.DeleteOper).WithPreCommitHook(r.deleteDefaultTelemetryPolicies)

	svc.GetCrudService("License", apiintf.CreateOper).GetRequestType().WithValidate(r.validateFFBootstrap)
	svc.GetCrudService("License", apiintf.UpdateOper).WithPreCommitHook(r.checkFFBootstrap)

	svc.GetCrudService("License", apiintf.CreateOper).WithPostCommitHook(r.applyFeatureFlags)
	svc.GetCrudService("License", apiintf.UpdateOper).WithPostCommitHook(r.applyFeatureFlags)
	apisrv.RegisterRestoreCallback(r.restoreFeatureFlags)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("cluster.ClusterV1", registerClusterHooks)
}
