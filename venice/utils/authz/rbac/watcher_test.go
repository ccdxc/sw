package rbac

import (
	"context"
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
)

const (
	apisrvURL = "localhost:0"
)

var (
	logger = log.WithContext("Pkg", "authz_watcher_test")

	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("authz_watcher_test", logger))
)

func createWatcher(cache *userPermissionsCache, name, apiSrvAddr string) *watcher {

	// create watcher
	watcher := newWatcher(cache, name, apiSrvAddr, nil)
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
	apiSrv, apiSrvAddr, err := serviceutils.StartAPIServer(apisrvURL, t.Name(), logger, []string{})
	AssertOk(t, err, "failed to create API server")
	// wait for api server to stop
	defer time.Sleep(time.Millisecond * 100)
	defer apiSrv.Stop()
	watcher := createWatcher(&userPermissionsCache{}, "watcher_test", apiSrvAddr)
	defer watcher.stop()
	apicl := createAPIClient(apiSrvAddr)
	defer apicl.Close()

	userPermCache := (watcher.cache).(*userPermissionsCache)
	// with no tenants created, role and role binding cache should be empty
	Assert(t, len(userPermCache.getRoleCache()) == 0, "tenant cache for roles is not empty")
	Assert(t, len(userPermCache.getRoleBindingCache()) == 0, "tenant cache for role bindings is not empty")

	// create tenant
	tenant := cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: "testtenant",
		},
	}
	_, err = apicl.ClusterV1().Tenant().Create(context.Background(), &tenant)
	AssertOk(t, err, "failed to create tenant")

	// verify the tenant cache got created
	AssertEventually(t, func() (bool, interface{}) {
		return len(userPermCache.getRoleCache()) == 1 && len(userPermCache.getRoleBindingCache()) == 1, nil
	}, "role and role binding cache not found for tenant")

	// create role
	role := auth.Role{
		TypeMeta: api.TypeMeta{Kind: "Role"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testtenant",
			Name:      "testRole",
			Namespace: globals.DefaultNamespace,
		},
		Spec: auth.RoleSpec{
			Permissions: []auth.Permission{
				{
					ResourceTenant:    "testtenant",
					ResourceGroup:     string(apiclient.GroupNetwork),
					ResourceKind:      string(network.KindNetwork),
					ResourceNamespace: "finance",
					Actions:           []string{auth.Permission_AllActions.String()},
				},
			},
		},
	}
	_, err = apicl.AuthV1().Role().Create(context.Background(), &role)
	AssertOk(t, err, "failed to create role")

	// verify the role got created in cache
	AssertEventually(t, func() (bool, interface{}) {
		cachedRole, ok := watcher.cache.getRole(role.GetName(), role.GetTenant())
		return ok && cachedRole.GetName() == role.GetName(), nil
	}, fmt.Sprintf("[%v] role not found", role.GetName()))

	// create role binding
	roleBinding := auth.RoleBinding{
		TypeMeta: api.TypeMeta{Kind: "RoleBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testtenant",
			Name:      "testRoleBinding",
			Namespace: globals.DefaultNamespace,
		},
		Spec: auth.RoleBindingSpec{
			Users:      []string{},
			UserGroups: []string{"testGroup"},
			Role:       "testRole",
		},
	}
	_, err = apicl.AuthV1().RoleBinding().Create(context.Background(), &roleBinding)
	AssertOk(t, err, "failed to create role binding")

	// verify the role binding got created in cache
	AssertEventually(t, func() (bool, interface{}) {
		cachedRoleBinding, ok := watcher.cache.getRoleBinding(roleBinding.GetName(), roleBinding.GetTenant())
		return ok && cachedRoleBinding.GetName() == roleBinding.GetName(), nil
	}, fmt.Sprintf("[%v] role binding not found", roleBinding.GetName()))

	// delete role binding
	_, err = apicl.AuthV1().RoleBinding().Delete(context.Background(), roleBinding.GetObjectMeta())
	AssertOk(t, err, "failed to delete role binding")

	// verify the role binding got deleted in cache
	AssertEventually(t, func() (bool, interface{}) {
		_, ok := watcher.cache.getRoleBinding(roleBinding.GetName(), roleBinding.GetTenant())
		return !ok, nil
	}, fmt.Sprintf("[%v] role binding not deleted", roleBinding.GetName()))

	// delete role
	_, err = apicl.AuthV1().Role().Delete(context.Background(), role.GetObjectMeta())
	AssertOk(t, err, "failed to delete role")

	// verify the role got deleted in cache
	AssertEventually(t, func() (bool, interface{}) {
		_, ok := watcher.cache.getRole(role.GetName(), role.GetTenant())
		return !ok, nil
	}, fmt.Sprintf("[%v] role not deleted", role.GetName()))

	// delete tenant
	_, err = apicl.ClusterV1().Tenant().Delete(context.Background(), tenant.GetObjectMeta())
	AssertOk(t, err, "failed to delete tenant")

	// verify the tenant cache got deleted
	AssertEventually(t, func() (bool, interface{}) {
		return len(userPermCache.getRoleCache()) == 0 && len(userPermCache.getRoleBindingCache()) == 0, nil
	}, "role and role binding cache not deleted for tenant")
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
			Tenant:    "testTenant",
			Name:      "testRole",
			Namespace: globals.DefaultNamespace,
		},
		Spec: auth.RoleSpec{
			Permissions: []auth.Permission{
				{
					ResourceTenant:    "testTenant",
					ResourceGroup:     string(apiclient.GroupNetwork),
					ResourceKind:      string(network.KindNetwork),
					ResourceNamespace: "finance",
					Actions:           []string{auth.Permission_AllActions.String()},
				},
			},
		},
	}
	testCache.addRole(role)
	_, ok := testCache.roles[role.GetTenant()][getKey(role.GetTenant(), role.GetName())]
	Assert(t, ok, "role didn't get added to the cache")
	apiSrv, apiSrvAddr, err := serviceutils.StartAPIServer(apisrvURL, t.Name(), logger, []string{})
	AssertOk(t, err, "failed to start API server")
	watcher := createWatcher(testCache, "watcher_test", apiSrvAddr)
	defer watcher.stop()
	Assert(t, !watcher.stopped(), "watcher shouldn't be in stopped state")
	// verify cache resets when watcher starts
	AssertEventually(t, func() (bool, interface{}) {
		userPermCache := (watcher.cache).(*userPermissionsCache)
		return len(userPermCache.getRoleCache()) == 0 && len(userPermCache.getRoleBindingCache()) == 0, nil
	}, "tenant cache for roles and role bindings should reset when watcher starts")

	// add role again
	testCache.addRole(role)
	_, ok = testCache.roles[role.GetTenant()][getKey(role.GetTenant(), role.GetName())]
	Assert(t, ok, "role didn't get added to the cache")

	// stop api server
	apiSrv.Stop()
	// wait for api server to stop
	time.Sleep(time.Millisecond * 100)
	Assert(t, !watcher.stopped(), "watcher shouldn't be in stopped state")
	// check if role is still there in cache when API server is down
	cachedRole, ok := watcher.cache.getRole(role.GetName(), role.GetTenant())
	Assert(t, ok && cachedRole.GetName() == role.GetName(), fmt.Sprintf("[%v] role not found in cache when api server stopped", role.GetName()))

	watcher.stop()
	Assert(t, watcher.stopped(), "watcher should be in stopped state")

}
