package rbac

import (
	"context"
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apiserver"
)

const (
	apisrvURL = "localhost:0"
)

func createAPIServer(url string) (apiserver.Server, string) {
	logger := log.WithContext("Pkg", "authz_watcher_test")

	// api server config
	sch := runtime.NewScheme()
	apisrvConfig := apiserver.Config{
		GrpcServerPort: url,
		Logger:         logger,
		Version:        "v1",
		Scheme:         sch,
		Kvstore: store.Config{
			Type:    store.KVStoreTypeMemkv,
			Servers: []string{""},
			Codec:   runtime.NewJSONCodec(sch),
		},
	}
	// create api server
	apiSrv := apisrvpkg.MustGetAPIServer()
	go apiSrv.Run(apisrvConfig)
	apiSrv.WaitRunning()

	if apiSrv == nil {
		panic("Unable to create API Server")
	}
	var err error
	apiSrvAddr, err := apiSrv.GetAddr()
	if err != nil {
		panic("Unable to get API Server address")
	}

	return apiSrv, apiSrvAddr
}

func createWatcher(cache *userPermissionsCache, name, apiSrvAddr string) *watcher {

	// create watcher
	watcher := newWatcher(cache, name, apiSrvAddr, "")
	return watcher
}

func createAPIClient(apiSrvAddr string) apiclient.Services {
	// api server client
	logger := log.WithContext("Pkg", "watcher_test")
	apicl, err := apiclient.NewGrpcAPIClient("watcher_test", apiSrvAddr, logger)
	if err != nil {
		panic("Error creating api client")
	}
	return apicl
}

func TestWatcher(t *testing.T) {
	apiSrv, apiSrvAddr := createAPIServer(apisrvURL)
	watcher := createWatcher(&userPermissionsCache{}, "watcher_test", apiSrvAddr)
	apicl := createAPIClient(apiSrvAddr)

	userPermCache := (watcher.cache).(*userPermissionsCache)
	// with no tenants created, role and role binding cache should be empty
	Assert(t, len(userPermCache.getRoleCache()) == 0, "tenant cache for roles is not empty")
	Assert(t, len(userPermCache.getRoleBindingCache()) == 0, "tenant cache for role bindings is not empty")

	// create tenant
	tenant := network.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testTenant",
		},
	}
	_, err := apicl.TenantV1().Tenant().Create(context.Background(), &tenant)
	AssertOk(t, err, "failed to create tenant")

	// verify the tenant cache got created
	AssertEventually(t, func() (bool, interface{}) {
		return len(userPermCache.getRoleCache()) == 1 && len(userPermCache.getRoleBindingCache()) == 1, nil
	}, "role and role binding cache not found for tenant")

	// create role
	role := auth.Role{
		TypeMeta: api.TypeMeta{Kind: "Role"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testRole",
		},
		Spec: auth.RoleSpec{
			Permissions: []auth.Permission{
				{
					ResourceGroup:     "Network",
					ResourceKind:      auth.Permission_NETWORK.String(),
					ResourceNamespace: "finance",
					Actions:           []string{auth.Permission_ALL_ACTIONS.String()},
				},
			},
		},
	}
	_, err = apicl.AuthV1().Role().Create(context.Background(), &role)
	AssertOk(t, err, "failed to create role")

	// verify the role got created in cache
	AssertEventually(t, func() (bool, interface{}) {
		cachedRole := watcher.cache.getRole(&api.ObjectMeta{Name: role.GetName(), Tenant: role.GetTenant()})
		return cachedRole != nil && cachedRole.GetName() == role.GetName(), nil
	}, fmt.Sprintf("[%v] role not found", role.GetName()))

	// create role binding
	roleBinding := auth.RoleBinding{
		TypeMeta: api.TypeMeta{Kind: "RoleBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testRoleBinding",
		},
		Spec: auth.RoleBindingSpec{
			Users:      []string{"testUser"},
			UserGroups: []string{"testGroup"},
			Role:       "testRole",
		},
	}
	_, err = apicl.AuthV1().RoleBinding().Create(context.Background(), &roleBinding)
	AssertOk(t, err, "failed to create role binding")

	// verify the role binding got created in cache
	AssertEventually(t, func() (bool, interface{}) {
		cachedRoleBinding := watcher.cache.getRoleBinding(&api.ObjectMeta{Name: roleBinding.GetName(), Tenant: roleBinding.GetTenant()})
		return cachedRoleBinding != nil && cachedRoleBinding.GetName() == roleBinding.GetName(), nil
	}, fmt.Sprintf("[%v] role binding not found", roleBinding.GetName()))

	// delete role binding
	_, err = apicl.AuthV1().RoleBinding().Delete(context.Background(), roleBinding.GetObjectMeta())
	AssertOk(t, err, "failed to delete role binding")

	// verify the role binding got deleted in cache
	AssertEventually(t, func() (bool, interface{}) {
		b := watcher.cache.getRoleBinding(&api.ObjectMeta{Name: roleBinding.GetName(), Tenant: roleBinding.GetTenant()})
		return b == nil, nil
	}, fmt.Sprintf("[%v] role binding not deleted", roleBinding.GetName()))

	// delete role
	_, err = apicl.AuthV1().Role().Delete(context.Background(), role.GetObjectMeta())
	AssertOk(t, err, "failed to delete role")

	// verify the role got deleted in cache
	AssertEventually(t, func() (bool, interface{}) {
		r := watcher.cache.getRole(&api.ObjectMeta{Name: role.GetName(), Tenant: role.GetTenant()})
		return r == nil, nil
	}, fmt.Sprintf("[%v] role not deleted", role.GetName()))

	// delete tenant
	_, err = apicl.TenantV1().Tenant().Delete(context.Background(), tenant.GetObjectMeta())
	AssertOk(t, err, "failed to delete tenant")

	// verify the tenant cache got deleted
	AssertEventually(t, func() (bool, interface{}) {
		return len(userPermCache.getRoleCache()) == 0 && len(userPermCache.getRoleBindingCache()) == 0, nil
	}, "role and role binding cache not deleted for tenant")

	apicl.Close()
	watcher.stop()
	apiSrv.Stop()
	// wait for api server to stop
	time.Sleep(time.Millisecond * 100)
}

func TestWatcherWithApiServerDown(t *testing.T) {
	testCache := &userPermissionsCache{
		roles:        make(map[string]map[string]*auth.Role),
		roleBindings: make(map[string]map[string]*auth.RoleBinding),
	}

	// add role
	role := &auth.Role{
		TypeMeta: api.TypeMeta{Kind: "Role"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testRole",
		},
		Spec: auth.RoleSpec{
			Permissions: []auth.Permission{
				{
					ResourceGroup:     "Network",
					ResourceKind:      auth.Permission_NETWORK.String(),
					ResourceNamespace: "finance",
					Actions:           []string{auth.Permission_ALL_ACTIONS.String()},
				},
			},
		},
	}
	testCache.addRole(role)
	cachedRole, ok := testCache.roles[role.GetTenant()][getKey(role.GetTenant(), role.GetName())]
	Assert(t, ok, "role didn't get added to the cache")

	apiSrv, apiSrvAddr := createAPIServer(apisrvURL)
	watcher := createWatcher(testCache, "watcher_test", apiSrvAddr)
	Assert(t, !watcher.stopped(), "watcher shouldn't be in stopped state")
	// verify cache resets when watcher starts
	AssertEventually(t, func() (bool, interface{}) {
		userPermCache := (watcher.cache).(*userPermissionsCache)
		return len(userPermCache.getRoleCache()) == 0 && len(userPermCache.getRoleBindingCache()) == 0, nil
	}, "tenant cache for roles and role bindings should reset when watcher starts")

	// add role again
	testCache.addRole(role)
	cachedRole, ok = testCache.roles[role.GetTenant()][getKey(role.GetTenant(), role.GetName())]
	Assert(t, ok, "role didn't get added to the cache")

	// stop api server
	apiSrv.Stop()
	// wait for api server to stop
	time.Sleep(time.Millisecond * 100)
	Assert(t, !watcher.stopped(), "watcher shouldn't be in stopped state")
	// check if role is still there in cache when API server is down
	cachedRole = watcher.cache.getRole(&api.ObjectMeta{Name: role.GetName(), Tenant: role.GetTenant()})
	Assert(t, cachedRole != nil && cachedRole.GetName() == role.GetName(), fmt.Sprintf("[%v] role not found in cache when api server stopped", role.GetName()))

	watcher.stop()
	Assert(t, watcher.stopped(), "watcher should be in stopped state")

}
