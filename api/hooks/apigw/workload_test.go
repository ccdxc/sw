package impl

import (
	"context"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/workload"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/login"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/utils/authz"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestWorkloadHooksRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwWorkloadHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &workloadHooks{}
	r.logger = l
	err := r.registerWorkloadHooks(svc)
	AssertOk(t, err, "apigw workload hook registration failed")
	prof, err := svc.GetCrudServiceProfile("Workload", apiintf.DeleteOper)
	AssertOk(t, err, "error getting service profile for Workload delete")
	Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre call hooks [%d] for workload delete profile", len(prof.PreCallHooks())))

	prof, err = svc.GetCrudServiceProfile("Workload", apiintf.CreateOper)
	AssertOk(t, err, "error getting service profile for Workload delete")
	Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre call hooks [%d] for workload create profile", len(prof.PreCallHooks())))

	prof, err = svc.GetCrudServiceProfile("Workload", apiintf.UpdateOper)
	AssertOk(t, err, "error getting service profile for Workload delete")
	Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre call hooks [%d] for workload update profile", len(prof.PreCallHooks())))

	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerWorkloadHooks(svc)
	Assert(t, err != nil, "expected error in workload hook registration")
}

func TestWorkloadUserContextHook(t *testing.T) {
	testUserRole := login.NewRole("UserRole", "testTenant",
		login.NewPermission(
			"testTenant",
			string(apiclient.GroupWorkload),
			string(workload.KindWorkload),
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_AllActions.String()))

	testUserRoleBinding := login.NewRoleBinding("UserRoleBinding", "testTenant", "UserRole", "testUser", "")
	tests := []struct {
		name          string
		user          *auth.User
		in            interface{}
		expectedPerms []auth.Permission
		expectedAdmin bool
		out           interface{}
		skipCall      bool
		err           bool
	}{
		{
			name: "Delete Workload",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			in: &workload.Workload{},
			expectedPerms: []auth.Permission{
				login.NewPermission(
					"testTenant",
					string(apiclient.GroupWorkload),
					string(workload.KindWorkload),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			expectedAdmin: false,
			out:           &workload.Workload{},
			skipCall:      false,
			err:           false,
		},
		{
			name: "Invalid input",
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			in:            &auth.User{},
			expectedPerms: []auth.Permission{},
			expectedAdmin: false,
			out:           nil,
			skipCall:      false,
			err:           true,
		},
	}
	r := &workloadHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwWorkloadHooks")
	r.logger = log.GetNewLogger(logConfig)
	r.permissionGetter = rbac.NewMockPermissionGetter([]*auth.Role{testUserRole}, []*auth.RoleBinding{testUserRoleBinding}, nil, nil)
	for _, test := range tests {
		nctx := apigwpkg.NewContextWithUser(context.TODO(), test.user)
		nctx, out, skipCall, err := r.userContext(nctx, test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		if !test.err {
			Assert(t, skipCall == test.skipCall, fmt.Sprintf("[%s] test failed", test.name))
			perms, _, _ := authzgrpcctx.PermsFromOutgoingContext(nctx)
			Assert(t, rbac.ArePermsEqual(test.expectedPerms, perms),
				fmt.Sprintf("[%s] test failed, expected perms [%s], got [%s]", test.name, rbac.PrintPerms(test.name, test.expectedPerms), rbac.PrintPerms(test.name, perms)))
			Assert(t, reflect.DeepEqual(test.out, out),
				fmt.Sprintf("[%s] test failed, expected returned object [%v], got [%v]", test.name, test.out, out))

			isAdmin, _ := authzgrpcctx.UserIsAdminFromOutgoingContext(nctx)
			Assert(t, reflect.DeepEqual(test.expectedAdmin, isAdmin),
				fmt.Sprintf("[%s] test failed, expected isAdmin to be [%v], got [%v]", test.name, test.expectedAdmin, isAdmin))
			Assert(t, reflect.DeepEqual(test.out, out),
				fmt.Sprintf("[%s] test failed, expected returned object [%v], got [%v]", test.name, test.out, out))
		}
	}
}

func TestWorkloadInterfaceValidation(t *testing.T) {
	tests := []struct {
		in     *workload.Workload
		expErr bool
	}{
		{
			in: &workload.Workload{
				Spec: workload.WorkloadSpec{
					Interfaces: []workload.WorkloadIntfSpec{
						workload.WorkloadIntfSpec{
							ExternalVlan: 0,
							Network:      "",
						},
					},
				},
			},
			expErr: false,
		},
		{
			in: &workload.Workload{
				Spec: workload.WorkloadSpec{
					Interfaces: []workload.WorkloadIntfSpec{
						workload.WorkloadIntfSpec{
							ExternalVlan: 0,
							Network:      "test",
						},
					},
				},
			},
			expErr: false,
		},
		{
			in: &workload.Workload{
				Spec: workload.WorkloadSpec{
					Interfaces: []workload.WorkloadIntfSpec{
						workload.WorkloadIntfSpec{
							ExternalVlan: 10,
							Network:      "",
						},
					},
				},
			},
			expErr: false,
		},
		{
			in: &workload.Workload{
				Spec: workload.WorkloadSpec{
					Interfaces: []workload.WorkloadIntfSpec{
						workload.WorkloadIntfSpec{
							ExternalVlan: 10,
							Network:      "",
						},
						workload.WorkloadIntfSpec{
							ExternalVlan: 10,
							Network:      "test",
						},
					},
				},
			},
			expErr: true,
		},
	}

	r := &workloadHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwWorkloadHooks")
	r.logger = log.GetNewLogger(logConfig)
	for i, tc := range tests {
		_, _, _, err := r.validateInterfaces(context.Background(), tc.in)
		if !tc.expErr {
			AssertOk(t, err, "tc %d failed", i)
		} else {
			Assert(t, err != nil, "tc %d: Expected err to be nil", i)
		}
	}
	// Test passing in bad type errs
	_, _, _, err := r.validateInterfaces(context.Background(), workload.WorkloadSpec{})
	Assert(t, err != nil, "Expected error when passing in bad type")

}
