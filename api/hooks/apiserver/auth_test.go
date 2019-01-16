package impl

import (
	"context"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"fmt"
	"net"
	"reflect"
	"sort"
	"testing"

	"github.com/pkg/errors"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/password"
	"github.com/pensando/sw/venice/utils/authz"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	testUser     = "test"
	testPassword = "pensandoo0"
)

func TestHashPassword(t *testing.T) {
	hasher := password.GetPasswordHasher()
	passwdhash, err := hasher.GetPasswordHash(testPassword)
	if err != nil {
		t.Fatalf("unable to hash password: %v", err)
	}
	tests := []struct {
		name     string
		oper     apiserver.APIOperType
		in       interface{}
		existing *auth.User
		result   bool
		err      error
	}{
		{
			name: "invalid input object",
			oper: apiserver.CreateOper,
			in: struct {
				Test string
			}{"testing"},
			result: false,
			err:    errInvalidInputType,
		},
		{
			name: "hash password for create user",
			oper: apiserver.CreateOper,
			in: auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:   testUser,
					Tenant: globals.DefaultTenant,
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: testPassword,
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			existing: nil,
			result:   true,
			err:      nil,
		},
		{
			name: "hash password for update user",
			oper: apiserver.UpdateOper,
			in: auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:   testUser,
					Tenant: globals.DefaultTenant,
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			existing: &auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:   testUser,
					Tenant: globals.DefaultTenant,
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: passwdhash,
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			result: true,
			err:    nil,
		},
		{
			name: "empty password for create user",
			oper: apiserver.CreateOper,
			in: auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:   testUser,
					Tenant: globals.DefaultTenant,
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			existing: nil,
			result:   false,
			err:      errEmptyPassword,
		},
		{
			name: "external user",
			oper: apiserver.CreateOper,
			in: auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:   testUser,
					Tenant: globals.DefaultTenant,
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: testPassword,
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_External.String(),
				},
			},
			existing: nil,
			result:   true,
			err:      nil,
		},
		{
			name: "user type not specified",
			oper: apiserver.CreateOper,
			in: auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Tenant: "default",
					Name:   testUser,
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: testPassword,
					Email:    "testuser@pensandio.io",
				},
			},
			existing: nil,
			result:   true,
			err:      nil,
		},
	}

	logConfig := log.GetDefaultConfig("TestAuthHooks")
	l := log.GetNewLogger(logConfig)
	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}
	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore %s", err)
	}

	authHooks := &authHooks{
		logger: l,
	}
	for _, test := range tests {
		ctx := context.TODO()
		txn := kvs.NewTxn()
		var userKey string
		if test.existing != nil {
			// encrypt password as it is stored as secret
			if err := test.existing.ApplyStorageTransformer(ctx, true); err != nil {
				t.Fatalf("[%s] test failed, error encrypting password, Err: %v", test.name, err)
			}
			userKey = test.existing.MakeKey("auth")
			if err := kvs.Create(ctx, userKey, test.existing); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with user, Err: %v", test.name, err)
			}
		}
		out, ok, err := authHooks.hashPassword(ctx, kvs, txn, userKey, test.oper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed, expected err [%v]. got [%v]", test.name, test.err, err))
		if err == nil {
			user, _ := out.(auth.User)
			if user.Spec.Type != auth.UserSpec_External.String() {
				ok, err := hasher.CompareHashAndPassword(user.Spec.Password, testPassword)
				AssertOk(t, err, fmt.Sprintf("[%v] test failed", test.name))
				Assert(t, ok, fmt.Sprintf("[%v] test failed", test.name))
			} else {
				Assert(t, user.Spec.Password == "", fmt.Sprintf("[%v] test failed, password should be empty for external user", test.name))
			}
		}
		kvs.Delete(ctx, userKey, nil)
	}
}

func TestValidateAuthenticatorConfigHook(t *testing.T) {
	tests := []struct {
		name string
		in   interface{}
		errs []error
	}{
		{
			name: "Missing LDAP config",
			in: auth.AuthenticationPolicy{
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
					TokenExpiry: "24h",
				},
			},
			errs: []error{errors.New("ldap authenticator config not defined")},
		},
		{
			name: "Missing Local config",
			in: auth.AuthenticationPolicy{
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
					TokenExpiry: "24h",
				},
			},
			errs: []error{errors.New("local authenticator config not defined")},
		},
		{
			name: "Missing Radius config",
			in: auth.AuthenticationPolicy{
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
					TokenExpiry: "24h",
				},
			},
			errs: []error{errors.New("radius authenticator config not defined")},
		},
		{
			name: "Missing AuthenticatorOrder",
			in: auth.AuthenticationPolicy{
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
					TokenExpiry: "24h",
				},
			},
			errs: []error{errors.New("authenticator order config not defined")},
		},
		{
			name: "no authenticator configs",
			in: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "MissingAuthenticatorsAuthenticationPolicy",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						AuthenticatorOrder: []string{},
					},
					TokenExpiry: "24h",
				},
			},
			errs: []error{errors.New("authenticator order config not defined")},
		},
		{
			name: "token expiry less than 2m",
			in: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "TokenExpiry1mAuthenticationPolicy",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Local: &auth.Local{
							Enabled: true,
						},
						AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String()},
					},
					TokenExpiry: "1m",
				},
			},
			errs: []error{fmt.Errorf("token expiry (%s) should be atleast 2 minutes", "1m")},
		},
		{
			name: "valid auth policy",
			in: auth.AuthenticationPolicy{
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
					TokenExpiry: "24h",
				},
			},
			errs: []error{},
		},
	}
	r := authHooks{}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	r.logger = log.GetNewLogger(logConfig)
	for _, test := range tests {
		errs := r.validateAuthenticatorConfig(test.in, "", false)
		sortErrors(errs)
		sortErrors(test.errs)
		Assert(t, len(errs) == len(test.errs), fmt.Sprintf("[%s] test failed, expected errors [%#v], got [%#v]", test.name, test.errs, errs))
		for i, err := range errs {
			Assert(t, err.Error() == test.errs[i].Error(), fmt.Sprintf("[%s] test failed, expected errors [%#v], got [%#v]", test.name, test.errs[i], errs[i]))
		}
	}
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

func TestChangePassword(t *testing.T) {
	hasher := password.GetPasswordHasher()
	passwdhash, err := hasher.GetPasswordHash(testPassword)
	if err != nil {
		t.Fatalf("unable to hash old password: %v", err)
	}
	const newPasswd = "pensando"
	tests := []struct {
		name     string
		in       interface{}
		existing *auth.User
		result   bool
		err      error
	}{
		{
			name: "invalid input object",
			in: struct {
				Test string
			}{"testing"},
			result: false,
			err:    errInvalidInputType,
		},
		{
			name: "change password",
			in:   auth.PasswordChangeRequest{OldPassword: testPassword, NewPassword: newPasswd},
			existing: &auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:         testUser,
					Tenant:       globals.DefaultTenant,
					GenerationID: "1",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: passwdhash,
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			result: false,
			err:    nil,
		},
		{
			name: "empty new password",
			in:   auth.PasswordChangeRequest{OldPassword: testPassword, NewPassword: ""},
			existing: &auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:         testUser,
					Tenant:       globals.DefaultTenant,
					GenerationID: "1",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: passwdhash,
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			result: false,
			err:    errEmptyPassword,
		},
		{
			name: "empty old password",
			in:   auth.PasswordChangeRequest{OldPassword: "", NewPassword: newPasswd},
			existing: &auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:         testUser,
					Tenant:       globals.DefaultTenant,
					GenerationID: "1",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: passwdhash,
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			result: false,
			err:    errEmptyPassword,
		},
		{
			name: "invalid old password",
			in:   auth.PasswordChangeRequest{OldPassword: "incorrectpasswd", NewPassword: newPasswd},
			existing: &auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:         testUser,
					Tenant:       globals.DefaultTenant,
					GenerationID: "1",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: passwdhash,
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			result: false,
			err:    errInvalidOldPassword,
		},
		{
			name: "external user",
			in:   auth.PasswordChangeRequest{OldPassword: testPassword, NewPassword: newPasswd},
			existing: &auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:         testUser,
					Tenant:       globals.DefaultTenant,
					GenerationID: "1",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: passwdhash,
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_External.String(),
				},
			},
			result: false,
			err:    errExtUserPasswordChange,
		},
	}

	logConfig := log.GetDefaultConfig("TestAuthHooks")
	l := log.GetNewLogger(logConfig)
	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}
	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore %s", err)
	}

	authHooks := &authHooks{
		logger: l,
	}
	for _, test := range tests {
		ctx := context.TODO()
		txn := kvs.NewTxn()
		var userKey string
		if test.existing != nil {
			// encrypt password as it is stored as secret
			if err := test.existing.ApplyStorageTransformer(ctx, true); err != nil {
				t.Fatalf("[%s] test failed, error encrypting password, Err: %v", test.name, err)
			}
			userKey = test.existing.MakeKey("auth")
			if err := kvs.Create(ctx, userKey, test.existing); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with user, Err: %v", test.name, err)
			}
		}
		_, ok, err := authHooks.changePassword(ctx, kvs, txn, userKey, "PasswordChange", false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed, expected err [%v]. got [%v]", test.name, test.err, err))
		if err == nil {
			req, _ := test.in.(auth.PasswordChangeRequest)
			user := &auth.User{}
			err = kvs.Get(ctx, userKey, user)
			AssertOk(t, err, fmt.Sprintf("[%v] test failed", test.name))
			err := user.ApplyStorageTransformer(context.Background(), false)
			AssertOk(t, err, fmt.Sprintf("[%v] test failed", test.name))
			ok, err := hasher.CompareHashAndPassword(user.Spec.Password, req.NewPassword)
			AssertOk(t, err, fmt.Sprintf("[%v] test failed", test.name))
			Assert(t, ok, fmt.Sprintf("[%v] test failed", test.name))
		}
		kvs.Delete(ctx, userKey, nil)
	}
}

func TestResetPassword(t *testing.T) {
	hasher := password.GetPasswordHasher()
	passwdhash, err := hasher.GetPasswordHash(testPassword)
	if err != nil {
		t.Fatalf("unable to hash old password: %v", err)
	}
	tests := []struct {
		name     string
		in       interface{}
		existing *auth.User
		result   bool
		err      error
	}{
		{
			name: "invalid input object",
			in: struct {
				Test string
			}{"testing"},
			result: false,
			err:    errInvalidInputType,
		},
		{
			name: "reset password",
			in:   auth.PasswordResetRequest{},
			existing: &auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:         testUser,
					Tenant:       globals.DefaultTenant,
					GenerationID: "1",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: passwdhash,
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			result: false,
			err:    nil,
		},
		{
			name: "external user",
			in:   auth.PasswordResetRequest{},
			existing: &auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:         testUser,
					Tenant:       globals.DefaultTenant,
					GenerationID: "1",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: passwdhash,
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_External.String(),
				},
			},
			result: false,
			err:    errExtUserPasswordChange,
		},
	}

	logConfig := log.GetDefaultConfig("TestAuthHooks")
	l := log.GetNewLogger(logConfig)
	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}
	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore %s", err)
	}

	authHooks := &authHooks{
		logger: l,
	}
	for _, test := range tests {
		ctx := context.TODO()
		txn := kvs.NewTxn()
		var userKey string
		if test.existing != nil {
			// encrypt password as it is stored as secret
			if err := test.existing.ApplyStorageTransformer(ctx, true); err != nil {
				t.Fatalf("[%s] test failed, error encrypting password, Err: %v", test.name, err)
			}
			userKey = test.existing.MakeKey("auth")
			if err := kvs.Create(ctx, userKey, test.existing); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with user, Err: %v", test.name, err)
			}
		}
		out, ok, err := authHooks.resetPassword(ctx, kvs, txn, userKey, "PasswordReset", false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed, expected err [%v]. got [%v]", test.name, test.err, err))
		if err == nil {
			user, _ := out.(auth.User)
			Assert(t, user.Spec.Password != test.existing.Spec.Password,
				fmt.Sprintf("[%v] test failed, reset password [%s] is not different than old password [%s]", test.name, user.Spec.Password, test.existing.Spec.Password))
		}
		kvs.Delete(ctx, userKey, nil)
	}
}

func TestPrivilegeEscalationCheck(t *testing.T) {
	tests := []struct {
		name         string
		ctx          context.Context
		in           interface{}
		role         *auth.Role
		user         *auth.User
		allowedPerms []auth.Permission
		result       bool
		err          error
	}{
		{
			name: "incorrect object type",
			ctx:  context.TODO(),
			in:   struct{ name string }{"testing"},
			role: login.NewRole("TestRole", globals.DefaultTenant,
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String())),
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: globals.DefaultTenant,
					Name:   "TestUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			allowedPerms: []auth.Permission{
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					authz.ResourceKindAll,
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			result: false,
			err:    errInvalidInputType,
		},
		{
			name: "valid case",
			in:   *login.NewRoleBinding("TestRoleBinding", globals.DefaultTenant, "TestRole", "TestUser", ""),
			role: login.NewRole("TestRole", globals.DefaultTenant,
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String())),
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: globals.DefaultTenant,
					Name:   "TestUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			allowedPerms: []auth.Permission{
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					authz.ResourceKindAll,
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			result: true,
			err:    nil,
		},
		{
			name: "privilege escalation",
			in:   *login.NewRoleBinding("TestRoleBinding", globals.DefaultTenant, "TestRole", "TestUser", ""),
			role: login.NewRole("TestRole", globals.DefaultTenant,
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					authz.ResourceKindAll,
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String())),
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: globals.DefaultTenant,
					Name:   "TestUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			allowedPerms: []auth.Permission{
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			result: false,
			err:    status.Error(codes.PermissionDenied, fmt.Sprintf("unauthorized to create role binding (%s|%s)", globals.DefaultTenant, "TestRoleBinding")),
		},
		{
			name: "no user in context",
			in:   *login.NewRoleBinding("TestRoleBinding", globals.DefaultTenant, "TestRole", "TestUser", ""),
			role: login.NewRole("TestRole", globals.DefaultTenant,
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String())),
			user: nil,
			allowedPerms: []auth.Permission{
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					authz.ResourceKindAll,
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			result: false,
			err:    status.Errorf(codes.Internal, "no user in context"),
		},
		{
			name: "no permissions",
			in:   *login.NewRoleBinding("TestRoleBinding", globals.DefaultTenant, "TestRole", "TestUser", ""),
			role: login.NewRole("TestRole", globals.DefaultTenant,
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String())),
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: globals.DefaultTenant,
					Name:   "TestUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			allowedPerms: []auth.Permission{},
			result:       false,
			err:          status.Error(codes.PermissionDenied, fmt.Sprintf("unauthorized to create role binding (%s|%s)", globals.DefaultTenant, "TestRoleBinding")),
		},
		{
			name: "nil permissions",
			in:   *login.NewRoleBinding("TestRoleBinding", globals.DefaultTenant, "TestRole", "TestUser", ""),
			role: login.NewRole("TestRole", globals.DefaultTenant,
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String())),
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: globals.DefaultTenant,
					Name:   "TestUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			allowedPerms: nil,
			result:       false,
			err:          status.Error(codes.PermissionDenied, fmt.Sprintf("unauthorized to create role binding (%s|%s)", globals.DefaultTenant, "TestRoleBinding")),
		},
		{
			name: "non existent role",
			in:   *login.NewRoleBinding("TestRoleBinding", globals.DefaultTenant, "TestRole", "TestUser", ""),
			role: login.NewRole("TestRole1", globals.DefaultTenant,
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					string(security.KindSGPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String())),
			user: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: globals.DefaultTenant,
					Name:   "TestUser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			allowedPerms: []auth.Permission{
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					authz.ResourceKindAll,
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			result: false,
			err:    kvstore.NewKeyNotFoundError("/venice/config/auth/roles/default/TestRole", 0),
		},
	}
	r := authHooks{}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	logConfig.Filter = log.AllowAllFilter
	r.logger = log.GetNewLogger(logConfig)
	addr := &net.IPAddr{
		IP: net.ParseIP("1.2.3.4"),
	}
	privateKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		t.Fatalf("error generating key: %v", err)
	}
	cert, err := certs.SelfSign(globals.APIGw, privateKey, certs.WithValidityDays(1))
	if err != nil {
		t.Fatalf("error generating certificate: %v", err)
	}
	ctx := ctxutils.MakeMockContext(addr, cert)
	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}
	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore: %v", err)
	}
	cluster := &cluster.Cluster{Status: cluster.ClusterStatus{AuthBootstrapped: true}}
	err = kvs.Create(ctx, cluster.MakeKey("cluster"), cluster)
	if err != nil {
		t.Fatalf("error populating cluster obj in kvstore: %v", err)
	}
	for _, test := range tests {
		nctx, err := authzgrpcctx.NewIncomingContextWithUserPerms(ctx, test.user, test.allowedPerms)
		err = kvs.Create(nctx, test.role.MakeKey("auth"), test.role)
		if err != nil {
			t.Fatalf("error creating test role [%#v]in kvstore: %v", test.role, err)
		}
		_, ok, err := r.privilegeEscalationCheck(nctx, kvs, nil, "", apiserver.CreateOper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed, expected err [%v]. got [%v]", test.name, test.err, err))
		kvs.Delete(nctx, test.role.MakeKey("auth"), nil)
	}
}

func sortErrors(errs []error) {
	sort.Slice(errs, func(i, j int) bool {
		return errs[i].Error() < errs[j].Error()
	})
}
