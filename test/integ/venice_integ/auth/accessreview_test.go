package auth

import (
	"context"
	"fmt"
	"reflect"
	"strings"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"

	. "github.com/pensando/sw/test/utils"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// test invalid action
// test invalid resource
// test empty/nil operation
// test authorized operation
func TestSubjectAccessReviewValidationFailures(t *testing.T) {
	tests := []struct {
		name     string
		op       *auth.Operation
		opStatus *auth.OperationStatus
	}{
		{
			name: "invalid action",
			op: &auth.Operation{
				Resource: &auth.Resource{
					Tenant: globals.DefaultTenant,
					Group:  string(apiclient.GroupAuth),
					Kind:   string(auth.KindRole),
				},
				Action: "invalid",
			},
			opStatus: &auth.OperationStatus{
				Operation: &auth.Operation{
					Resource: &auth.Resource{
						Tenant: globals.DefaultTenant,
						Group:  string(apiclient.GroupAuth),
						Kind:   string(auth.KindRole),
					},
					Action: "invalid",
				},
				Allowed: false,
				Message: ".Action did not match allowed strings",
			},
		},
		{
			name: "invalid resource",
			op: &auth.Operation{
				Resource: &auth.Resource{
					Tenant: "testtenant",
					Group:  string(apiclient.GroupAuth),
					Kind:   string(auth.KindAuthenticationPolicy),
				},
				Action: auth.Permission_Create.String(),
			},
			opStatus: &auth.OperationStatus{
				Operation: &auth.Operation{
					Resource: &auth.Resource{
						Tenant: "testtenant",
						Group:  string(apiclient.GroupAuth),
						Kind:   string(auth.KindAuthenticationPolicy),
					},
					Action: auth.Permission_Create.String(),
				},
				Allowed: false,
				Message: "tenant should be empty or [\"default\"] for cluster scoped resource kind [\"AuthenticationPolicy\"]",
			},
		},
		{
			name: "empty operation",
			op:   &auth.Operation{},
			opStatus: &auth.OperationStatus{
				Operation: &auth.Operation{},
				Allowed:   false,
				Message:   "resource not specified",
			},
		},
		{
			name: "nil operation",
			op:   nil,
			opStatus: &auth.OperationStatus{
				Operation: nil,
				Allowed:   false,
				Message:   "operation not specified",
			},
		},
		{
			name: "authorized tenant scoped operation",
			op: &auth.Operation{
				Resource: &auth.Resource{
					Tenant: globals.DefaultTenant,
					Group:  string(apiclient.GroupAuth),
					Kind:   string(auth.KindRole),
				},
				Action: auth.Permission_Create.String(),
			},
			opStatus: &auth.OperationStatus{
				Operation: &auth.Operation{
					Resource: &auth.Resource{
						Tenant: globals.DefaultTenant,
						Group:  string(apiclient.GroupAuth),
						Kind:   string(auth.KindRole),
					},
					Action: auth.Permission_Create.String(),
				},
				Allowed: true,
				Message: "",
			},
		},
		{
			name: "authorized cluster scoped operation",
			op: &auth.Operation{
				Resource: &auth.Resource{
					Tenant: "",
					Group:  string(apiclient.GroupAuth),
					Kind:   string(auth.KindAuthenticationPolicy),
				},
				Action: auth.Permission_Create.String(),
			},
			opStatus: &auth.OperationStatus{
				Operation: &auth.Operation{
					Resource: &auth.Resource{
						Tenant: "",
						Group:  string(apiclient.GroupAuth),
						Kind:   string(auth.KindAuthenticationPolicy),
					},
					Action: auth.Permission_Create.String(),
				},
				Allowed: true,
				Message: "",
			},
		},
		{
			name: "authorized cluster scoped operation with default tenant specified",
			op: &auth.Operation{
				Resource: &auth.Resource{
					Tenant: globals.DefaultTenant,
					Group:  string(apiclient.GroupAuth),
					Kind:   string(auth.KindAuthenticationPolicy),
				},
				Action: auth.Permission_Create.String(),
			},
			opStatus: &auth.OperationStatus{
				Operation: &auth.Operation{
					Resource: &auth.Resource{
						Tenant: globals.DefaultTenant,
						Group:  string(apiclient.GroupAuth),
						Kind:   string(auth.KindAuthenticationPolicy),
					},
					Action: auth.Permission_Create.String(),
				},
				Allowed: true,
				Message: "",
			},
		},
	}
	adminCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create default tenant and global admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, adminCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, adminCred, tinfo.l)

	superAdminCtx, err := NewLoggedInContext(context.Background(), tinfo.apiGwAddr, adminCred)
	AssertOk(t, err, "error creating logged in context")
	for _, test := range tests {
		var user *auth.User
		AssertEventually(t, func() (bool, interface{}) {
			user, err = tinfo.restcl.AuthV1().User().IsAuthorized(superAdminCtx, &auth.SubjectAccessReviewRequest{
				ObjectMeta: api.ObjectMeta{Name: testUser, Tenant: globals.DefaultTenant},
				Operations: []*auth.Operation{test.op},
			})
			return err == nil, err
		}, "IsAuthorized call failed")
		Assert(t, len(user.Status.AccessReview) == 1, fmt.Sprintf("[%s] test failed, unexpected number of operations in status [%v]", test.name, len(user.Status.AccessReview)))
		opStatus := user.Status.AccessReview[0]
		Assert(t, strings.HasPrefix(opStatus.Message, test.opStatus.Message), fmt.Sprintf("[%s] test failed, expected op status message[%s], got [%s]", test.name, test.opStatus.Message, opStatus.Message))
		Assert(t, opStatus.Allowed == test.opStatus.Allowed, fmt.Sprintf("[%s] test failed, expected op status [%v], got [%v]", test.name, test.opStatus.Allowed, opStatus.Allowed))
	}
}

// test for self user access
func TestSelfSubjectAccessReview(t *testing.T) {
	const (
		testUser2 = "testUser2"
		testUser3 = "testUser3"
	)
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	MustCreateTestUser(tinfo.apicl, testUser2, testPassword, testTenant)
	defer MustDeleteUser(tinfo.apicl, testUser2, testTenant)
	MustCreateTestUser(tinfo.apicl, testUser3, testPassword, testTenant)
	defer MustDeleteUser(tinfo.apicl, testUser3, testTenant)
	// login as testUser2 who has no roles assigned
	ctx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, &auth.PasswordCredential{Username: testUser2, Password: testPassword, Tenant: testTenant})
	AssertOk(t, err, "error creating logged in context")
	tests := []struct {
		name     string
		op       *auth.Operation
		opStatus *auth.OperationStatus
	}{
		{
			name: "user self get",
			op: &auth.Operation{
				Resource: &auth.Resource{
					Tenant: testTenant,
					Group:  string(apiclient.GroupAuth),
					Kind:   string(auth.KindUser),
					Name:   testUser2,
				},
				Action: auth.Permission_Read.String(),
			},
			opStatus: &auth.OperationStatus{
				Operation: &auth.Operation{
					Resource: &auth.Resource{
						Tenant: testTenant,
						Group:  string(apiclient.GroupAuth),
						Kind:   string(auth.KindUser),
						Name:   testUser2,
					},
					Action: auth.Permission_Read.String(),
				},
				Allowed: true,
				Message: "",
			},
		},
		{
			name: "user self update",
			op: &auth.Operation{
				Resource: &auth.Resource{
					Tenant: testTenant,
					Group:  string(apiclient.GroupAuth),
					Kind:   string(auth.KindUser),
					Name:   testUser2,
				},
				Action: auth.Permission_Update.String(),
			},
			opStatus: &auth.OperationStatus{
				Operation: &auth.Operation{
					Resource: &auth.Resource{
						Tenant: testTenant,
						Group:  string(apiclient.GroupAuth),
						Kind:   string(auth.KindUser),
						Name:   testUser2,
					},
					Action: auth.Permission_Update.String(),
				},
				Allowed: true,
				Message: "",
			},
		},
		{
			name: "user self action",
			op: &auth.Operation{
				Resource: &auth.Resource{
					Tenant: testTenant,
					Group:  string(apiclient.GroupAuth),
					Kind:   string(auth.KindUser),
					Name:   testUser2,
				},
				Action: auth.Permission_Create.String(),
			},
			opStatus: &auth.OperationStatus{
				Operation: &auth.Operation{
					Resource: &auth.Resource{
						Tenant: testTenant,
						Group:  string(apiclient.GroupAuth),
						Kind:   string(auth.KindUser),
						Name:   testUser2,
					},
					Action: auth.Permission_Create.String(),
				},
				Allowed: true,
				Message: "",
			},
		},
		{
			name: "check non-self user action",
			op: &auth.Operation{
				Resource: &auth.Resource{
					Tenant: testTenant,
					Group:  string(apiclient.GroupAuth),
					Kind:   string(auth.KindUser),
					Name:   testUser3,
				},
				Action: auth.Permission_Create.String(),
			},
			opStatus: &auth.OperationStatus{
				Operation: &auth.Operation{
					Resource: &auth.Resource{
						Tenant: testTenant,
						Group:  string(apiclient.GroupAuth),
						Kind:   string(auth.KindUser),
						Name:   testUser3,
					},
					Action: auth.Permission_Create.String(),
				},
				Allowed: false,
				Message: "",
			},
		},
	}
	for _, test := range tests {
		var user *auth.User
		AssertEventually(t, func() (bool, interface{}) {
			user, err = tinfo.restcl.AuthV1().User().IsAuthorized(ctx, &auth.SubjectAccessReviewRequest{
				ObjectMeta: api.ObjectMeta{Name: testUser2, Tenant: testTenant},
				Operations: []*auth.Operation{
					test.op,
				},
			})
			return err == nil, err
		}, "IsAuthorized action failed")
		Assert(t, len(user.Status.AccessReview) == 1, fmt.Sprintf("[%s] test failed, unexpected number of operations in status [%v]", test.name, len(user.Status.AccessReview)))
		opStatus := user.Status.AccessReview[0]
		Assert(t, reflect.DeepEqual(test.opStatus.Operation, opStatus.Operation),
			fmt.Sprintf("[%s] test failed, expected status for operation [%#v], got [%#v]", test.name, test.opStatus.Operation, opStatus.Operation))
		Assert(t, opStatus.Message == test.opStatus.Message, fmt.Sprintf("[%s] test failed, expected op status message to be [%s], got [%s]", test.name, test.opStatus.Message, opStatus.Message))
		Assert(t, opStatus.Allowed == test.opStatus.Allowed, fmt.Sprintf("[%s] test failed, expected user authorization to be [%v], got [%v]", test.name, test.opStatus.Allowed, opStatus.Allowed))
	}
}

// test for non-existent user
func TestNonExistentUser(t *testing.T) {
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)
	ctx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, &auth.PasswordCredential{Username: testUser, Password: testPassword, Tenant: testTenant})
	AssertOk(t, err, "error creating logged in context")
	_, err = tinfo.restcl.AuthV1().User().IsAuthorized(ctx, &auth.SubjectAccessReviewRequest{
		ObjectMeta: api.ObjectMeta{Name: "nonexistent", Tenant: testTenant},
		Operations: []*auth.Operation{
			{
				Resource: &auth.Resource{
					Tenant: testTenant,
					Group:  string(apiclient.GroupAuth),
					Kind:   string(auth.KindRole),
					Name:   globals.AdminRole,
				},
				Action: auth.Permission_Read.String(),
			},
		},
	})
	Assert(t, err != nil, "expected request for nonexistent user to fail")
}

// test authorized, unauthorized operation for other user
func TestSubjectAccessReview(t *testing.T) {
	// test for authorized and unauthorized access for a different user
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)
	ctx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, &auth.PasswordCredential{Username: testUser, Password: testPassword, Tenant: testTenant})
	// create testUser2 as network admin
	const (
		testUser2 = "testUser2"
	)
	MustCreateTestUser(tinfo.apicl, testUser2, testPassword, testTenant)
	defer MustDeleteUser(tinfo.apicl, testUser2, testTenant)
	MustCreateRole(tinfo.apicl, "NetworkAdminRole", testTenant, login.NewPermission(
		testTenant,
		string(apiclient.GroupNetwork),
		string(network.KindNetwork),
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_AllActions.String()))
	defer MustDeleteRole(tinfo.apicl, "NetworkAdminRole", testTenant)
	MustCreateRoleBinding(tinfo.apicl, "NetworkAdminRoleBinding", testTenant, "NetworkAdminRole", []string{testUser2}, nil)
	defer MustDeleteRoleBinding(tinfo.apicl, "NetworkAdminRoleBinding", testTenant)
	tests := []struct {
		name     string
		op       *auth.Operation
		opStatus *auth.OperationStatus
	}{
		{
			name: "network read",
			op: &auth.Operation{
				Resource: &auth.Resource{
					Tenant: testTenant,
					Group:  string(apiclient.GroupNetwork),
					Kind:   string(network.KindNetwork),
					Name:   "network1",
				},
				Action: auth.Permission_Read.String(),
			},
			opStatus: &auth.OperationStatus{
				Operation: &auth.Operation{
					Resource: &auth.Resource{
						Tenant: testTenant,
						Group:  string(apiclient.GroupNetwork),
						Kind:   string(network.KindNetwork),
						Name:   "network1",
					},
					Action: auth.Permission_Read.String(),
				},
				Allowed: true,
				Message: "",
			},
		},
		{
			name: "role read",
			op: &auth.Operation{
				Resource: &auth.Resource{
					Tenant: testTenant,
					Group:  string(apiclient.GroupAuth),
					Kind:   string(auth.KindRole),
					Name:   globals.AdminRole,
				},
				Action: auth.Permission_Read.String(),
			},
			opStatus: &auth.OperationStatus{
				Operation: &auth.Operation{
					Resource: &auth.Resource{
						Tenant: testTenant,
						Group:  string(apiclient.GroupAuth),
						Kind:   string(auth.KindRole),
						Name:   globals.AdminRole,
					},
					Action: auth.Permission_Read.String(),
				},
				Allowed: false,
				Message: "",
			},
		},
		{
			name: "user self read",
			op: &auth.Operation{
				Resource: &auth.Resource{
					Tenant: testTenant,
					Group:  string(apiclient.GroupAuth),
					Kind:   string(auth.KindUser),
					Name:   testUser2,
				},
				Action: auth.Permission_Read.String(),
			},
			opStatus: &auth.OperationStatus{
				Operation: &auth.Operation{
					Resource: &auth.Resource{
						Tenant: testTenant,
						Group:  string(apiclient.GroupAuth),
						Kind:   string(auth.KindUser),
						Name:   testUser2,
					},
					Action: auth.Permission_Read.String(),
				},
				Allowed: true,
				Message: "",
			},
		},
	}
	for _, test := range tests {
		var user *auth.User
		AssertEventually(t, func() (bool, interface{}) {
			user, err = tinfo.restcl.AuthV1().User().IsAuthorized(ctx, &auth.SubjectAccessReviewRequest{
				ObjectMeta: api.ObjectMeta{Name: testUser2, Tenant: testTenant},
				Operations: []*auth.Operation{test.op},
			})
			return err == nil, err
		}, "IsAuthorized call failed")
		Assert(t, len(user.Status.AccessReview) == 1, fmt.Sprintf("[%s] test failed, unexpected number of operations in status [%v]", test.name, len(user.Status.AccessReview)))
		opStatus := user.Status.AccessReview[0]
		Assert(t, reflect.DeepEqual(test.opStatus.Operation, opStatus.Operation),
			fmt.Sprintf("[%s] test failed, expected status for operation [%#v], got [%#v]", test.name, test.opStatus.Operation, opStatus.Operation))
		Assert(t, strings.HasPrefix(opStatus.Message, test.opStatus.Message), fmt.Sprintf("[%s] test failed, expected op status message[%s], got [%s]", test.name, test.opStatus.Message, opStatus.Message))
		Assert(t, opStatus.Allowed == test.opStatus.Allowed, fmt.Sprintf("[%s] test failed, expected op status [%v], got [%v]", test.name, test.opStatus.Allowed, opStatus.Allowed))
	}
}
