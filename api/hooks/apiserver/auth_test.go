package impl

import (
	"context"
	"fmt"
	"testing"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/authn/password"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	testUser     = "test"
	testPassword = "pensandoo0"
)

func runPasswordHook(oper apiserver.APIOperType, userType auth.UserSpec_UserType, password string) (interface{}, bool, error) {
	r := authHooks{}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	r.logger = log.GetNewLogger(logConfig)
	// user object
	user := auth.User{
		TypeMeta: api.TypeMeta{Kind: "User"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   testUser,
		},
		Spec: auth.UserSpec{
			Fullname: "Test User",
			Password: password,
			Email:    "testuser@pensandio.io",
			Type:     userType.String(),
		},
	}

	return r.hashPassword(context.Background(), nil, nil, "", oper, false, user)
}

func TestWithCreateOperation(t *testing.T) {
	i, ok, err := runPasswordHook(apiserver.CreateOper, auth.UserSpec_Local, testPassword)

	Assert(t, ok, "hook to hash password failed for create")
	AssertOk(t, err, "Error hashing password in create pre-commit hook")

	// get password hasher
	hasher := password.GetPasswordHasher()
	// compare hash
	ok, err = hasher.CompareHashAndPassword(i.(auth.User).Spec.Password, testPassword)
	AssertOk(t, err, "Error comparing password hash")
	Assert(t, ok, "Password didn't match")

}

func TestWithUpdateOperation(t *testing.T) {
	i, ok, err := runPasswordHook(apiserver.UpdateOper, auth.UserSpec_Local, testPassword)

	Assert(t, ok, "hook to hash password failed for create")
	AssertOk(t, err, "Error hashing password in create pre-commit hook")

	// get password hasher
	hasher := password.GetPasswordHasher()
	// compare hash
	ok, err = hasher.CompareHashAndPassword(i.(auth.User).Spec.Password, testPassword)
	AssertOk(t, err, "Error comparing password hash")
	Assert(t, ok, "Password didn't match")
}

func TestForExternalUser(t *testing.T) {
	i, ok, err := runPasswordHook(apiserver.CreateOper, auth.UserSpec_External, "")

	Assert(t, ok, "hook to hash password failed for external user")
	AssertOk(t, err, "Error hashing password in create pre-commit hook for external user")
	Assert(t, i.(auth.User).Spec.Password == "", "Password shouldn't be hashed for external user")
}

func TestWithDeleteOperation(t *testing.T) {
	i, ok, err := runPasswordHook(apiserver.DeleteOper, auth.UserSpec_Local, testPassword)

	Assert(t, ok, "hook to hash password failed for delete")
	AssertOk(t, err, "Error returned for delete operation")
	Assert(t, i.(auth.User).Spec.Password == testPassword, "hook to hash password should be no-op for delete operation")
}

func TestWithEmptyPasswordForUpdateOperation(t *testing.T) {
	i, ok, err := runPasswordHook(apiserver.UpdateOper, auth.UserSpec_Local, "")

	Assert(t, !ok, "hook to hash password succeeded for empty password in update operation")
	Assert(t, err != nil, "No error returned for empty password in update operation")
	Assert(t, i.(auth.User).Spec.Password == "", "Empty password shouldn't be hashed in update operation")
}

func TestWithEmptyPasswordForCreateOperation(t *testing.T) {
	i, ok, err := runPasswordHook(apiserver.CreateOper, auth.UserSpec_Local, "")

	Assert(t, !ok, "hook to hash password succeeded for empty password in create operation")
	Assert(t, err != nil, "No error returned for empty password in create operation")
	Assert(t, i.(auth.User).Spec.Password == "", "Empty password shouldn't be hashed in create operation")
}

func TestWithInvalidInputObject(t *testing.T) {
	r := authHooks{}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	r.logger = log.GetNewLogger(logConfig)

	_, ok, err := r.hashPassword(context.Background(), nil, nil, "", apiserver.CreateOper, false, r)
	Assert(t, !ok, "hook to hash password succeeded for invalid input")
	Assert(t, err != nil, "No error returned for invalid input")
}

func TestWithUserTypeNotSpecified(t *testing.T) {
	r := authHooks{}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	r.logger = log.GetNewLogger(logConfig)

	// user object
	user := auth.User{
		TypeMeta: api.TypeMeta{Kind: "User"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   testUser,
		},
		Spec: auth.UserSpec{
			Fullname: "Test User",
			Password: testPassword,
			Email:    "testuser@pensandio.io",
		},
	}

	i, ok, err := r.hashPassword(context.Background(), nil, nil, "", apiserver.CreateOper, false, user)
	user = i.(auth.User)
	Assert(t, ok, "hook to hash password failed when user type not specified")
	Assert(t, user.Spec.Type == auth.UserSpec_Local.String(), "user type should default to local")
	AssertOk(t, err, "Error returned when user type not specified")
}

// erraneousAuthenticatorsConfig returns test data for testing incorrect Authenticators configuration given to validateAuthenticationPolicy hook
func erraneousAuthenticatorsConfig() map[string]*auth.AuthenticationPolicy {
	policydata := make(map[string]*auth.AuthenticationPolicy)

	policydata["Missing LDAP config"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "MissingLDAPAuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
		},
	}
	policydata["Missing Local config"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "MissingLocalAuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
		},
	}
	policydata["Missing Radius config"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "MissingRadiusAuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: true,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_LDAP.String(), auth.Authenticators_RADIUS.String()},
			},
		},
	}
	policydata["Missing AuthenticatorOrder"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "MissingAuthenticatorOrderAuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: true,
				},
				Local: &auth.Local{
					Enabled: true,
				},
			},
		},
	}
	policydata["Missing Authenticators config"] = &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "MissingAuthenticatorsAuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{},
	}

	return policydata
}

func validAuthenticationPolicyData() *auth.AuthenticationPolicy {
	return &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: false,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
		},
	}
}

func TestValidateAuthenticatorConfigHook(t *testing.T) {
	r := authHooks{}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	r.logger = log.GetNewLogger(logConfig)
	// test for missing authenticator config
	for testtype, policy := range erraneousAuthenticatorsConfig() {
		err := r.validateAuthenticatorConfig(*policy, "", false)
		Assert(t, err != nil, fmt.Sprintf("[%v] No error returned for mis-configured Authenticators", testtype))
		Assert(t, err[0] == errAuthenticatorConfig, fmt.Sprintf("[%v] Unexpected error returned for mis-configured Authenticators: Err: %v", testtype, err))
	}

	// test for correctly configured  authentication policy
	policy := validAuthenticationPolicyData()
	errs := r.validateAuthenticatorConfig(*policy, "", false)
	Assert(t, len(errs) == 0, "Validation hook failed for correctly configured authenticators")
}

func TestGenerateSecret(t *testing.T) {
	tests := []struct {
		name string
		in   interface{}
		ok   bool
		err  bool
	}{
		{
			name: "incorrect object type",
			in:   struct{ name string }{"testing"},
			ok:   false,
			err:  true,
		},
		{
			name: "successful secret generation",
			in: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "AuthenticationPolicy",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Local: &auth.Local{
							Enabled: true,
						},
						AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String()},
					},
				},
			},
			ok:  true,
			err: false,
		},
	}
	r := authHooks{}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	r.logger = log.GetNewLogger(logConfig)

	for _, test := range tests {
		_, ok, err := r.generateSecret(context.Background(), nil, nil, "", apiserver.CreateOper, false, test.in)
		Assert(t, (test.ok == ok) && (test.err == (err != nil)), fmt.Sprintf("[%s] test failed", test.name))
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
	}
}

func TestValidateRolePerms(t *testing.T) {
	tests := []struct {
		name string
		in   interface{}
		err  []error
	}{
		{
			name: "valid role",
			in: *login.NewRole("NetworkAdminRole",
				"testTenant",
				login.NewPermission("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					"",
					"",
					auth.Permission_AllActions.String())),
			err: nil,
		},
		{
			name: "valid cluster role",
			in: *login.NewClusterRole("ClusterAdminRole",
				login.NewPermission("testTenant",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					"",
					"",
					auth.Permission_AllActions.String())),
			err: nil,
		},
		{
			name: "invalid role",
			in: *login.NewRole("NetworkAdminRole",
				"testTenant",
				login.NewPermission("default",
					string(apiclient.GroupNetwork),
					string(network.KindNetwork),
					"",
					"",
					auth.Permission_AllActions.String())),
			err: []error{errInvalidRolePermissions},
		},
		{
			name: "invalid resource kind in a resource group",
			in: *login.NewRole("NetworkAdminRole",
				"testTenant",
				login.NewPermission("testTenant",
					string(apiclient.GroupNetwork),
					string(auth.KindUser),
					"",
					"",
					auth.Permission_AllActions.String())),
			err: []error{fmt.Errorf("invalid resource kind [%q] and API group [%q]", string(auth.KindUser), string(apiclient.GroupNetwork))},
		},
		{
			name: "no resource group and empty kind",
			in: *login.NewRole("AdminRole",
				"testTenant",
				login.NewPermission("testTenant",
					"",
					"",
					"",
					"",
					auth.Permission_AllActions.String())),
			err: []error{fmt.Errorf("invalid API group [%q]", "")},
		},
		{
			name: "no resource group and all resource kinds",
			in: *login.NewRole("AdminRole",
				"testTenant",
				login.NewPermission("testTenant",
					"",
					authz.ResourceKindAll,
					"",
					"",
					auth.Permission_AllActions.String())),
			err: []error{fmt.Errorf("invalid API group [%q]", "")},
		},
		{
			name: "all resource group and all resource kinds",
			in: *login.NewRole("AdminRole",
				"testTenant",
				login.NewPermission("testTenant",
					authz.ResourceGroupAll,
					authz.ResourceKindAll,
					"",
					"",
					auth.Permission_AllActions.String())),
			err: nil,
		},
		{
			name: "all resource group and empty resource kinds",
			in: *login.NewRole("AdminRole",
				"testTenant",
				login.NewPermission("testTenant",
					authz.ResourceGroupAll,
					"",
					"",
					"",
					auth.Permission_AllActions.String())),
			err: nil,
		},
		{
			name: "role with search endpoint permission",
			in: *login.NewRole("SearchRole",
				"testTenant",
				login.NewPermission("testTenant",
					"",
					auth.Permission_Search.String(),
					"",
					"",
					auth.Permission_AllActions.String())),
			err: nil,
		},
		{
			name: "role with invalid search endpoint permission",
			in: *login.NewRole("SearchRole",
				"testTenant",
				login.NewPermission("testTenant",
					authz.ResourceGroupAll,
					auth.Permission_Search.String(),
					"",
					"",
					auth.Permission_AllActions.String())),
			err: []error{fmt.Errorf("invalid API group, should be empty instead of [%q]", authz.ResourceGroupAll)},
		},
		{
			name: "role with event endpoint permission",
			in: *login.NewRole("EventRole",
				"testTenant",
				login.NewPermission("testTenant",
					"",
					auth.Permission_Event.String(),
					"",
					"",
					auth.Permission_AllActions.String())),
			err: nil,
		},
		{
			name: "role with invalid event endpoint permission",
			in: *login.NewRole("EventRole",
				"testTenant",
				login.NewPermission("testTenant",
					string(apiclient.GroupMonitoring),
					auth.Permission_Event.String(),
					"",
					"",
					auth.Permission_AllActions.String())),
			err: []error{fmt.Errorf("invalid API group, should be empty instead of [%q]", string(apiclient.GroupMonitoring))},
		},
		{
			name: "role with api endpoint permission",
			in: *login.NewRole("APIRole",
				"testTenant",
				login.NewPermission("testTenant",
					"",
					auth.Permission_APIEndpoint.String(),
					"",
					"/api/v1/search",
					auth.Permission_AllActions.String())),
			err: nil,
		},
		{
			name: "role with api endpoint permission having no resource name",
			in: *login.NewRole("APIRole",
				"testTenant",
				login.NewPermission("testTenant",
					"",
					auth.Permission_APIEndpoint.String(),
					"",
					"",
					auth.Permission_AllActions.String())),
			err: []error{fmt.Errorf("missing API endpoint resource name")},
		},
		{
			name: "role with api endpoint permission having group name",
			in: *login.NewRole("APIRole",
				"testTenant",
				login.NewPermission("testTenant",
					string(apiclient.GroupMonitoring),
					auth.Permission_APIEndpoint.String(),
					"",
					"",
					auth.Permission_AllActions.String())),
			err: []error{fmt.Errorf("invalid API group, should be empty instead of [%q]", string(apiclient.GroupMonitoring))},
		},
		{
			name: "incorrect object type",
			in:   struct{ name string }{"testing"},
			err:  []error{errors.New("invalid input type")},
		},
	}
	r := authHooks{}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	r.logger = log.GetNewLogger(logConfig)
	for _, test := range tests {
		err := r.validateRolePerms(test.in, "", false)

		Assert(t, func() bool {
			if err == nil {
				return test.err == nil
			}
			if test.err == nil {
				return false
			}
			return err[0].Error() == test.err[0].Error()
		}(), fmt.Sprintf("[%s] test failed", test.name))
	}
}
