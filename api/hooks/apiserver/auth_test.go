package impl

import (
	"context"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"errors"
	"fmt"
	"net"
	"reflect"
	"testing"
	"time"

	"google.golang.org/grpc/codes"
	k8serrors "k8s.io/apimachinery/pkg/util/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/password"
	"github.com/pensando/sw/venice/utils/authz"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	testUser     = "test"
	testPassword = "Pensandoo0%"
)

var (
	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("auth_test",
		log.GetNewLogger(log.GetDefaultConfig("auth_test"))))
)

func TestHashPassword(t *testing.T) {
	hasher := password.GetPasswordHasher()
	passwdhash, err := hasher.GetPasswordHash(testPassword)
	if err != nil {
		t.Fatalf("unable to hash password: %v", err)
	}
	tests := []struct {
		name     string
		oper     apiintf.APIOperType
		in       interface{}
		existing *auth.User
		result   bool
		err      error
	}{
		{
			name: "invalid input object",
			oper: apiintf.CreateOper,
			in: struct {
				Test string
			}{"testing"},
			result: true,
			err:    errInvalidInputType,
		},
		{
			name: "hash password for create user",
			oper: apiintf.CreateOper,
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
			oper: apiintf.UpdateOper,
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
			oper: apiintf.CreateOper,
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
			result:   true,
			err:      errEmptyPassword,
		},
		{
			name: "external user",
			oper: apiintf.CreateOper,
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
			oper: apiintf.CreateOper,
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
		{
			name: "non compliant password",
			oper: apiintf.CreateOper,
			in: auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:   testUser,
					Tenant: globals.DefaultTenant,
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "Aabcdefg12",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			existing: nil,
			result:   true,
			err:      k8serrors.NewAggregate([]error{password.ErrInsufficientSymbols}),
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
						AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String()},
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
						Local: &auth.Local{
							Enabled: true,
						},
						AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_RADIUS.String()},
					},
					TokenExpiry: "24h",
				},
			},
			errs: []error{errors.New("radius authenticator config not defined")},
		},
		{
			name: "Invalid Radius config",
			in: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "InvalidRadiusAuthenticationPolicy",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Local: &auth.Local{
							Enabled: true,
						},
						Radius: &auth.Radius{
							Enabled: true,
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{
											Url: "",
										},
									},
								},
							},
						},
						AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_RADIUS.String()},
					},
					TokenExpiry: "24h",
				},
			},
			errs: []error{errors.New("NAS ID cannot be empty or longer than 253 bytes"),
				errors.New("radius <address:port> not defined")},
		},
		{
			name: "Missing Radius server",
			in: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "MissingRadiusServerAuthenticationPolicy",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Local: &auth.Local{
							Enabled: true,
						},
						Radius: &auth.Radius{
							Enabled: true,
							Domains: []*auth.RadiusDomain{
								{
									NasID: "Venice",
								},
							},
						},
						AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_RADIUS.String()},
					},
					TokenExpiry: "24h",
				},
			},
			errs: []error{errors.New("radius server not defined")},
		},
		{
			name: "Valid Radius config",
			in: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "ValidRadiusServerAuthenticationPolicy",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Local: &auth.Local{
							Enabled: true,
						},
						Radius: &auth.Radius{
							Enabled: true,
							Domains: []*auth.RadiusDomain{
								{
									NasID: "Venice",
									Servers: []*auth.RadiusServer{
										{
											Url: "localhost:1812",
										},
									},
								},
							},
						},
						AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_RADIUS.String()},
					},
					TokenExpiry: "24h",
				},
			},
			errs: []error{},
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
		errs := r.validateAuthenticatorConfig(test.in, "", false, false)
		SortErrors(errs)
		SortErrors(test.errs)
		Assert(t, len(errs) == len(test.errs), fmt.Sprintf("[%s] test failed, expected errors [%#v], got [%#v]", test.name, test.errs, errs))
		for i, err := range errs {
			Assert(t, err.Error() == test.errs[i].Error(), fmt.Sprintf("[%s] test failed, expected errors [%#v], got [%#v]", test.name, test.errs[i], errs[i]))
		}
	}
}

func TestValidateBindPassword(t *testing.T) {
	tests := []struct {
		name string
		in   interface{}
		err  error
	}{
		{
			name: "missing bind password",
			in: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "AuthenticationPolicy",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Enabled: true,
							Domains: []*auth.LdapDomain{
								{
									Servers: []*auth.LdapServer{
										{
											Url: "localhost:389",
											TLSOptions: &auth.TLSOptions{
												StartTLS:                   true,
												SkipServerCertVerification: true,
											},
										},
									},

									BaseDN:       "DC=pensando,DC=io",
									BindDN:       "CN=admin,DC=pensando,DC=io",
									BindPassword: "",
									AttributeMapping: &auth.LdapAttributeMapping{
										User:             "uid",
										UserObjectClass:  "inetPersonOrg",
										Group:            "memberOf",
										GroupObjectClass: "group",
									},
								},
							},
						},
						Local: &auth.Local{
							Enabled: true,
						},
						AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
					},
					TokenExpiry: "24h",
				},
			},
			err: errors.New("bind password not defined"),
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
			err: nil,
		},
		{
			name: "invalid input object",
			in: struct {
				Test string
			}{"testing"},
			err: errInvalidInputType,
		},
	}
	r := authHooks{}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	r.logger = log.GetNewLogger(logConfig)
	for _, test := range tests {
		_, _, err := r.validateBindPassword(context.Background(), nil, nil, "", apiintf.CreateOper, false, test.in)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, got error [%v], expected [%v]", test.name, err, test.err))
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
			ok:   true,
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
		_, ok, err := r.generateSecret(context.Background(), nil, nil, "", apiintf.CreateOper, false, test.in)
		Assert(t, (test.ok == ok) && (test.err == (err != nil)), fmt.Sprintf("[%s] test failed", test.name))
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
	}
}

func TestGenerateSecretAction(t *testing.T) {
	tests := []struct {
		name     string
		in       interface{}
		existing *auth.AuthenticationPolicy
		out      auth.AuthenticationPolicy
		result   bool
		err      error
	}{
		{
			name: "invalid input object",
			in: struct {
				Test string
			}{"testing"},
			result: true,
			err:    fmt.Errorf("invalid input type"),
		},
		{
			name: "valid generate secret request",
			in:   auth.TokenSecretRequest{},
			existing: &auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "1",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			out: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "2",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			result: false,
			err:    nil,
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
		var policyKey string
		if test.existing != nil {
			// encrypt password as it is stored as secret
			if err := test.existing.ApplyStorageTransformer(ctx, true); err != nil {
				t.Fatalf("[%s] test failed, error encrypting secret fields, Err: %v", test.name, err)
			}
			policyKey = test.existing.MakeKey("auth")
			if err := kvs.Create(ctx, policyKey, test.existing); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with policy, Err: %v", test.name, err)
			}
		}
		_, ok, err := authHooks.generateSecretAction(ctx, kvs, txn, policyKey, apiintf.CreateOper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed, expected err [%v]. got [%v]", test.name, test.err, err))
		if err == nil {
			policy := &auth.AuthenticationPolicy{}
			err = kvs.Get(ctx, policyKey, policy)
			AssertOk(t, err, fmt.Sprintf("[%v] test failed", test.name))
			err := policy.ApplyStorageTransformer(context.Background(), false)
			AssertOk(t, err, fmt.Sprintf("[%v] test failed", test.name))
			Assert(t, policy.Spec.Authenticators.Ldap.Domains[0].BindPassword == test.out.Spec.Authenticators.Ldap.Domains[0].BindPassword,
				fmt.Sprintf("[%v] test failed, expected bind password [%s], got [%s]", test.name, test.out.Spec.Authenticators.Ldap.Domains[0].BindPassword, policy.Spec.Authenticators.Ldap.Domains[0].BindPassword))
			Assert(t, len(policy.Spec.Authenticators.Radius.Domains[0].Servers) == len(test.out.Spec.Authenticators.Radius.Domains[0].Servers),
				fmt.Sprintf("[%v] test failed, expected radius server count [%d], got [%d]", test.name, len(test.out.Spec.Authenticators.Radius.Domains[0].Servers), len(policy.Spec.Authenticators.Radius.Domains[0].Servers)))
			for _, radius := range policy.Spec.Authenticators.Radius.Domains[0].Servers {
				for _, expectedRadius := range test.out.Spec.Authenticators.Radius.Domains[0].Servers {
					if radius.Url == expectedRadius.Url {
						Assert(t, radius.Secret == expectedRadius.Secret,
							fmt.Sprintf("[%v] test failed, expected radius [%s] secret [%s], got [%s]", test.name, radius.Url, expectedRadius.Secret, radius.Secret))
					}
				}
			}
			Assert(t, len(policy.Spec.Secret) == 128, fmt.Sprintf("[%v] test failed, expected secret length 128, got [%d]", test.name, len(policy.Spec.Secret)))
		}
		kvs.Delete(ctx, policyKey, nil)
	}
}

func TestPopulateSecretsInAuthPolicy(t *testing.T) {
	tests := []struct {
		name     string
		oper     apiintf.APIOperType
		in       interface{}
		existing *auth.AuthenticationPolicy
		result   bool
		out      auth.AuthenticationPolicy
		err      error
	}{
		{
			name: "invalid input object",
			oper: apiintf.UpdateOper,
			in: struct {
				Test string
			}{"testing"},
			result: true,
			err:    errInvalidInputType,
		},
		{
			name: "update auth policy without secrets",
			oper: apiintf.UpdateOper,
			in: auth.AuthenticationPolicy{
				TypeMeta:   api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812"},
										{Url: "192.168.10.12:1812"},
									},
								},
							},
						},
					},
				},
			},
			existing: &auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "1",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			result: false,
			out: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "2",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			err: nil,
		},
		{
			name: "update auth policy with secrets",
			oper: apiintf.UpdateOper,
			in: auth.AuthenticationPolicy{
				TypeMeta:   api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: "newpassword",
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: "newpassword"},
										{Url: "192.168.10.12:1812", Secret: "newpassword"},
									},
								},
							},
						},
					},
				},
			},
			existing: &auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "1",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			result: false,
			out: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "2",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: "newpassword",
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: "newpassword"},
										{Url: "192.168.10.12:1812", Secret: "newpassword"},
									},
								},
							},
						},
					},
				},
			},
			err: nil,
		},
		{
			name: "update auth policy with some secrets",
			oper: apiintf.UpdateOper,
			in: auth.AuthenticationPolicy{
				TypeMeta:   api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{{}},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812"},
										{Url: "192.168.10.12:1812", Secret: "newpassword"},
									},
								},
							},
						},
					},
				},
			},
			existing: &auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "1",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			result: false,
			out: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "2",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: "newpassword"},
									},
								},
							},
						},
					},
				},
			},
			err: nil,
		},
		{
			name: "remove a radius server",
			oper: apiintf.UpdateOper,
			in: auth.AuthenticationPolicy{
				TypeMeta:   api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{{}},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.12:1812", Secret: "newpassword"},
									},
								},
							},
						},
					},
				},
			},
			existing: &auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "1",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			result: false,
			out: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "2",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.12:1812", Secret: "newpassword"},
									},
								},
							},
						},
					},
				},
			},
			err: nil,
		},
		{
			name: "add a radius server",
			oper: apiintf.UpdateOper,
			in: auth.AuthenticationPolicy{
				TypeMeta:   api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{{}},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812"},
										{Url: "192.168.10.12:1812"},
										{Url: "192.168.10.13:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			existing: &auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "1",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			result: false,
			out: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "2",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: testPassword},
										{Url: "192.168.10.13:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			err: nil,
		},
		{
			name: "non-existent auth policy",
			oper: apiintf.UpdateOper,
			in: auth.AuthenticationPolicy{
				TypeMeta:   api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.13:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			existing: nil,
			result:   true,
			err:      kvstore.NewKeyNotFoundError("", 0),
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
		var policyKey string
		if test.existing != nil {
			// encrypt password as it is stored as secret
			if err := test.existing.ApplyStorageTransformer(ctx, true); err != nil {
				t.Fatalf("[%s] test failed, error encrypting secret fields, Err: %v", test.name, err)
			}
			policyKey = test.existing.MakeKey("auth")
			if err := kvs.Create(ctx, policyKey, test.existing); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with policy, Err: %v", test.name, err)
			}
		}
		out, ok, err := authHooks.populateSecretsInAuthPolicy(ctx, kvs, txn, policyKey, test.oper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed, expected err [%v]. got [%v]", test.name, test.err, err))
		if err == nil {
			policy, _ := out.(auth.AuthenticationPolicy)
			Assert(t, policy.Spec.Authenticators.Ldap.Domains[0].BindPassword == test.out.Spec.Authenticators.Ldap.Domains[0].BindPassword,
				fmt.Sprintf("[%v] test failed, expected bind password [%s], got [%s]", test.name, test.out.Spec.Authenticators.Ldap.Domains[0].BindPassword, policy.Spec.Authenticators.Ldap.Domains[0].BindPassword))
			Assert(t, len(policy.Spec.Authenticators.Radius.Domains[0].Servers) == len(test.out.Spec.Authenticators.Radius.Domains[0].Servers),
				fmt.Sprintf("[%v] test failed, expected radius server count [%d], got [%d]", test.name, len(test.out.Spec.Authenticators.Radius.Domains[0].Servers), len(policy.Spec.Authenticators.Radius.Domains[0].Servers)))
			for _, radius := range policy.Spec.Authenticators.Radius.Domains[0].Servers {
				for _, expectedRadius := range test.out.Spec.Authenticators.Radius.Domains[0].Servers {
					if radius.Url == expectedRadius.Url {
						Assert(t, radius.Secret == expectedRadius.Secret,
							fmt.Sprintf("[%v] test failed, expected radius [%s] secret [%s], got [%s]", test.name, radius.Url, expectedRadius.Secret, radius.Secret))
					}
				}
			}
		}
		kvs.Delete(ctx, policyKey, nil)
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
					auth.Permission_Read.String())),
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
					auth.Permission_Read.String())),
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
					auth.Permission_Read.String())),
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
					auth.Permission_Read.String())),
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
		err := r.validateRolePerms(test.in, "", false, false)

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
	const newPasswd = "NewPensando0$"
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
			result: true,
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
			result: true,
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
			result: true,
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
			result: true,
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
			result: true,
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
		currtime := time.Now()
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
			chngpasswdtime, err := user.Status.LastPasswordChange.Time()
			AssertOk(t, err, "error getting password change time")
			Assert(t, chngpasswdtime.After(currtime), fmt.Sprintf("password change time [%v] not after current time [%v]", chngpasswdtime.Local(), currtime.Local()))
			Assert(t, chngpasswdtime.Sub(currtime) < 30*time.Second, fmt.Sprintf("password change time [%v] not within 30 seconds of current time [%v]", chngpasswdtime, currtime))
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
			result: true,
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
			result: true,
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
		currtime := time.Now()
		_, ok, err := authHooks.resetPassword(ctx, kvs, txn, userKey, "PasswordReset", false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed, expected err [%v]. got [%v]", test.name, test.err, err))
		if err == nil {
			user := &auth.User{}
			err = kvs.Get(ctx, userKey, user)
			AssertOk(t, err, fmt.Sprintf("[%v] test failed", test.name))
			Assert(t, user.Spec.Password != test.existing.Spec.Password,
				fmt.Sprintf("[%v] test failed, reset password [%s] is not different than old password [%s]", test.name, user.Spec.Password, test.existing.Spec.Password))
			chngpasswdtime, err := user.Status.LastPasswordChange.Time()
			AssertOk(t, err, "error getting password change time")
			Assert(t, chngpasswdtime.After(currtime), fmt.Sprintf("password change time [%v] not after current time [%v]", chngpasswdtime.Local(), currtime.Local()))
			Assert(t, chngpasswdtime.Sub(currtime) < 30*time.Second, fmt.Sprintf("password change time [%v] not within 30 seconds of current time [%v]", chngpasswdtime, currtime))
		}
		kvs.Delete(ctx, userKey, nil)
	}
}

func TestPrivilegeEscalationCheck(t *testing.T) {
	tests := []struct {
		name         string
		in           interface{}
		role         *auth.Role
		user         *auth.User
		allowedPerms []auth.Permission
		result       bool
		err          error
	}{
		{
			name: "incorrect object type",
			in:   struct{ name string }{"testing"},
			role: login.NewRole("TestRole", globals.DefaultTenant,
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					string(security.KindNetworkSecurityPolicy),
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
			err:    errInvalidInputType,
		},
		{
			name: "valid case",
			in:   *login.NewRoleBinding("TestRoleBinding", globals.DefaultTenant, "TestRole", "TestUser", ""),
			role: login.NewRole("TestRole", globals.DefaultTenant,
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					string(security.KindNetworkSecurityPolicy),
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
					string(security.KindNetworkSecurityPolicy),
					authz.ResourceNamespaceAll,
					"",
					auth.Permission_AllActions.String()),
			},
			result: true,
			err: &api.Status{
				TypeMeta: api.TypeMeta{Kind: "Status"},
				Message:  []string{fmt.Sprintf("unauthorized to create role binding (%s|%s)", globals.DefaultTenant, "TestRoleBinding")},
				Code:     int32(codes.PermissionDenied),
				Result:   api.StatusResult{Str: "Authorization failed"},
			},
		},
		{
			name: "no user in context",
			in:   *login.NewRoleBinding("TestRoleBinding", globals.DefaultTenant, "TestRole", "TestUser", ""),
			role: login.NewRole("TestRole", globals.DefaultTenant,
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					string(security.KindNetworkSecurityPolicy),
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
			result: true,
			err: &api.Status{
				TypeMeta: api.TypeMeta{Kind: "Status"},
				Message:  []string{"no user in context"},
				Code:     int32(codes.Internal),
				Result:   api.StatusResult{Str: "Internal error"},
			},
		},
		{
			name: "no permissions",
			in:   *login.NewRoleBinding("TestRoleBinding", globals.DefaultTenant, "TestRole", "TestUser", ""),
			role: login.NewRole("TestRole", globals.DefaultTenant,
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					string(security.KindNetworkSecurityPolicy),
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
			result:       true,
			err: &api.Status{
				TypeMeta: api.TypeMeta{Kind: "Status"},
				Message:  []string{fmt.Sprintf("unauthorized to create role binding (%s|%s)", globals.DefaultTenant, "TestRoleBinding")},
				Code:     int32(codes.PermissionDenied),
				Result:   api.StatusResult{Str: "Authorization failed"},
			},
		},
		{
			name: "nil permissions",
			in:   *login.NewRoleBinding("TestRoleBinding", globals.DefaultTenant, "TestRole", "TestUser", ""),
			role: login.NewRole("TestRole", globals.DefaultTenant,
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					string(security.KindNetworkSecurityPolicy),
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
			result:       true,
			err: &api.Status{
				TypeMeta: api.TypeMeta{Kind: "Status"},
				Message:  []string{fmt.Sprintf("unauthorized to create role binding (%s|%s)", globals.DefaultTenant, "TestRoleBinding")},
				Code:     int32(codes.PermissionDenied),
				Result:   api.StatusResult{Str: "Authorization failed"},
			},
		},
		{
			name: "non existent role",
			in:   *login.NewRoleBinding("TestRoleBinding", globals.DefaultTenant, "TestRole", "TestUser", ""),
			role: login.NewRole("TestRole1", globals.DefaultTenant,
				login.NewPermission(globals.DefaultTenant,
					string(apiclient.GroupSecurity),
					string(security.KindNetworkSecurityPolicy),
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
		_, ok, err := r.privilegeEscalationCheck(nctx, kvs, nil, "", apiintf.CreateOper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed, expected err [%v]. got [%v]", test.name, test.err, err))
		kvs.Delete(nctx, test.role.MakeKey("auth"), nil)
	}
}

func TestValidatePassword(t *testing.T) {
	tests := []struct {
		name   string
		in     interface{}
		errors []error
	}{
		{
			"user create",
			auth.User{
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
			nil,
		},
		{
			"change password",
			auth.PasswordChangeRequest{OldPassword: "asdfa34345@", NewPassword: testPassword},
			nil,
		},
		{
			"incorrect object type",
			struct{ name string }{"testing"},
			[]error{errInvalidInputType},
		},
	}
	r := authHooks{}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	r.logger = log.GetNewLogger(logConfig)
	for _, test := range tests {
		errs := r.validatePassword(test.in, "", false, false)
		SortErrors(errs)
		SortErrors(test.errors)
		Assert(t, reflect.DeepEqual(errs, test.errors), fmt.Sprintf("%s test failed, expected errors [%v], got [%v]", test.name, test.errors, errs))
	}
}

func TestAdminRoleCheck(t *testing.T) {
	testSuperAdminRole := login.NewClusterRole(globals.AdminRole, login.NewPermission(
		authz.ResourceTenantAll,
		authz.ResourceGroupAll,
		authz.ResourceKindAll,
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_AllActions.String()))
	testNetworkAdminRole := login.NewRole("NetworkAdmin", "testTenant", login.NewPermission(
		"testTenant",
		string(apiclient.GroupNetwork),
		string(network.KindNetwork),
		authz.ResourceNamespaceAll,
		"network1,network2",
		fmt.Sprintf("%s,%s,%s", auth.Permission_Create.String(), auth.Permission_Update.String(), auth.Permission_Delete.String())),
		login.NewPermission(
			"testTenant",
			string(apiclient.GroupNetwork),
			string(network.KindLbPolicy),
			authz.ResourceNamespaceAll,
			"",
			fmt.Sprintf("%s,%s,%s", auth.Permission_Create.String(), auth.Permission_Update.String(), auth.Permission_Delete.String())))

	tests := []struct {
		name   string
		in     interface{}
		out    interface{}
		result bool
		err    error
	}{
		{
			name:   "super admin role",
			in:     *testSuperAdminRole,
			out:    *testSuperAdminRole,
			result: true,
			err:    errAdminRoleUpdateNotAllowed,
		},
		{
			name:   "network admin role",
			in:     *testNetworkAdminRole,
			out:    *testNetworkAdminRole,
			result: true,
			err:    nil,
		},
		{
			name:   "incorrect object type",
			in:     struct{ name string }{"testing"},
			out:    struct{ name string }{"testing"},
			result: true,
			err:    errInvalidInputType,
		},
	}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l

	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}
	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore: %v", err)
	}
	for _, test := range tests {
		ctx := context.TODO()
		txn := kvs.NewTxn()
		out, result, err := r.adminRoleCheck(ctx, kvs, txn, "", apiintf.CreateOper, false, test.in)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected err [%v], got [%v]", test.name, test.err, err))
		Assert(t, result == test.result, fmt.Sprintf("[%s] test failed, expected result [%v], got [%v]", test.name, test.result, result))
		Assert(t, reflect.DeepEqual(out, test.out), fmt.Sprintf("[%s] test failed, expected obj [%v], got [%v]", test.name, test.out, out))
	}
}

func TestAdminRoleBindingCheck(t *testing.T) {
	adminRoleBinding := login.NewClusterRoleBinding(globals.AdminRoleBinding, globals.AdminRole, "", "")
	networkAdminRoleBinding := login.NewRoleBinding("NetworkAdminRb", "testtenant", "NetworkAdmin", "", "")
	incorrectAdminRoleBinding := login.NewClusterRoleBinding(globals.AdminRoleBinding, "nonAdminRole", "", "")
	tests := []struct {
		name   string
		in     interface{}
		oper   apiintf.APIOperType
		out    interface{}
		result bool
		err    error
	}{
		{
			name:   "super admin role",
			in:     *adminRoleBinding,
			oper:   apiintf.DeleteOper,
			out:    *adminRoleBinding,
			result: true,
			err:    errAdminRoleBindingDeleteNotAllowed,
		},
		{
			name:   "network admin role",
			in:     *networkAdminRoleBinding,
			oper:   apiintf.DeleteOper,
			out:    *networkAdminRoleBinding,
			result: true,
			err:    nil,
		},
		{
			name:   "incorrect object type",
			in:     struct{ name string }{"testing"},
			oper:   apiintf.DeleteOper,
			out:    struct{ name string }{"testing"},
			result: true,
			err:    errInvalidInputType,
		},
		{
			name:   "updating with AdminRole name",
			in:     *adminRoleBinding,
			oper:   apiintf.UpdateOper,
			out:    *adminRoleBinding,
			result: true,
			err:    nil,
		},
		{
			name:   "updating with not AdminRole name",
			in:     *incorrectAdminRoleBinding,
			oper:   apiintf.UpdateOper,
			out:    *incorrectAdminRoleBinding,
			result: true,
			err:    errAdminRoleBindingRoleUpdateNotAllowed,
		},
	}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l

	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}
	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore: %v", err)
	}
	for _, test := range tests {
		ctx := context.TODO()
		txn := kvs.NewTxn()
		out, result, err := r.adminRoleBindingCheck(ctx, kvs, txn, "", test.oper, false, test.in)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected err [%v], got [%v]", test.name, test.err, err))
		Assert(t, result == test.result, fmt.Sprintf("[%s] test failed, expected result [%v], got [%v]", test.name, test.result, result))
		Assert(t, reflect.DeepEqual(out, test.out), fmt.Sprintf("[%s] test failed, expected obj [%v], got [%v]", test.name, test.out, out))
	}
}

func TestReturnAuthPolicy(t *testing.T) {
	tests := []struct {
		name     string
		in       interface{}
		existing *auth.AuthenticationPolicy
		out      auth.AuthenticationPolicy
		err      error
	}{
		{
			name: "existing auth policy",
			in: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "1",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			existing: &auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "1",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			out: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "2",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			err: nil,
		}, {
			name: "no existing auth policy",
			in: auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindAuthenticationPolicy)},
				ObjectMeta: api.ObjectMeta{
					GenerationID: "1",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Domains: []*auth.LdapDomain{
								{
									BindPassword: testPassword,
								},
							},
						},
						Radius: &auth.Radius{
							Domains: []*auth.RadiusDomain{
								{
									Servers: []*auth.RadiusServer{
										{Url: "192.168.10.11:1812", Secret: testPassword},
										{Url: "192.168.10.12:1812", Secret: testPassword},
									},
								},
							},
						},
					},
				},
			},
			err: kvstore.NewKeyNotFoundError("/venice/config/auth/authn-policy/Singleton", 0),
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
		var policyKey string
		if test.existing != nil {
			// encrypt password as it is stored as secret
			if err := test.existing.ApplyStorageTransformer(ctx, true); err != nil {
				t.Fatalf("[%s] test failed, error encrypting secret fields, Err: %v", test.name, err)
			}
			policyKey = test.existing.MakeKey("auth")
			if err := kvs.Create(ctx, policyKey, test.existing); err != nil {
				t.Fatalf("[%s] test failed, unable to populate kvstore with policy, Err: %v", test.name, err)
			}
		}

		out, err := authHooks.returnAuthPolicy(ctx, kvs, "", nil, nil, test.in, apiintf.CreateOper)
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed, expected err [%v]. got [%v]", test.name, test.err, err))
		if err == nil {
			policy, _ := out.(auth.AuthenticationPolicy)
			Assert(t, policy.Spec.Authenticators.Ldap.Domains[0].BindPassword == test.out.Spec.Authenticators.Ldap.Domains[0].BindPassword,
				fmt.Sprintf("[%v] test failed, expected bind password [%s], got [%s]", test.name, test.out.Spec.Authenticators.Ldap.Domains[0].BindPassword, policy.Spec.Authenticators.Ldap.Domains[0].BindPassword))
			Assert(t, len(policy.Spec.Authenticators.Radius.Domains[0].Servers) == len(test.out.Spec.Authenticators.Radius.Domains[0].Servers),
				fmt.Sprintf("[%v] test failed, expected radius server count [%d], got [%d]", test.name, len(test.out.Spec.Authenticators.Radius.Domains[0].Servers), len(policy.Spec.Authenticators.Radius.Domains[0].Servers)))
			for _, radius := range policy.Spec.Authenticators.Radius.Domains[0].Servers {
				for _, expectedRadius := range test.out.Spec.Authenticators.Radius.Domains[0].Servers {
					if radius.Url == expectedRadius.Url {
						Assert(t, radius.Secret == expectedRadius.Secret,
							fmt.Sprintf("[%v] test failed, expected radius [%s] secret [%s], got [%s]", test.name, radius.Url, expectedRadius.Secret, radius.Secret))
					}
				}
			}
		}
		kvs.Delete(ctx, policyKey, nil)
	}
}

func TestReturnUser(t *testing.T) {
	hasher := password.GetPasswordHasher()
	passwdhash, err := hasher.GetPasswordHash(testPassword)
	if err != nil {
		t.Fatalf("unable to hash old password: %v", err)
	}
	tests := []struct {
		name     string
		in       interface{}
		existing *auth.User
		out      auth.User
		err      error
	}{
		{
			name: "existing user",
			in: auth.User{
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
			existing: &auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:            testUser,
					Tenant:          globals.DefaultTenant,
					ResourceVersion: "1",
					GenerationID:    "1",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: passwdhash,
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			out: auth.User{
				TypeMeta: api.TypeMeta{Kind: string(auth.KindUser)},
				ObjectMeta: api.ObjectMeta{
					Name:            testUser,
					Tenant:          globals.DefaultTenant,
					ResourceVersion: "1",
					GenerationID:    "1",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: passwdhash,
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_Local.String(),
				},
			},
			err: nil,
		},
		{
			name: "non existent user",
			in: auth.User{
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
			err: kvstore.NewKeyNotFoundError("/venice/config/auth/users/default/test", 0),
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
		out, err := authHooks.returnUser(ctx, kvs, "", nil, nil, test.in, apiintf.CreateOper)
		Assert(t, reflect.DeepEqual(test.err, err), fmt.Sprintf("[%v] test failed, expected err [%v]. got [%v]", test.name, test.err, err))
		if err == nil {
			user, _ := out.(auth.User)
			// overwrite the creation time and mod time
			user.ModTime, user.CreationTime = test.out.ModTime, test.out.CreationTime
			Assert(t, reflect.DeepEqual(user, test.out), fmt.Sprintf("[%v] test failed, expected user [%#v], got [%#v]", test.name, test.out, user))
		}
		kvs.Delete(ctx, userKey, nil)
	}

}

func TestPermissionTenantCheck(t *testing.T) {
	testSuperAdminRole := login.NewClusterRole(globals.AdminRole, login.NewPermission(
		authz.ResourceTenantAll,
		authz.ResourceGroupAll,
		authz.ResourceKindAll,
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_AllActions.String()))
	testNetworkAdminRole := login.NewRole("NetworkAdmin", "testTenant", login.NewPermission(
		"testTenant",
		string(apiclient.GroupNetwork),
		string(network.KindNetwork),
		authz.ResourceNamespaceAll,
		"network1,network2",
		fmt.Sprintf("%s,%s,%s", auth.Permission_Create.String(), auth.Permission_Update.String(), auth.Permission_Delete.String())),
		login.NewPermission(
			"testTenant",
			string(apiclient.GroupNetwork),
			string(network.KindLbPolicy),
			authz.ResourceNamespaceAll,
			"",
			fmt.Sprintf("%s,%s,%s", auth.Permission_Create.String(), auth.Permission_Update.String(), auth.Permission_Delete.String())))

	tests := []struct {
		name   string
		in     interface{}
		out    interface{}
		result bool
		err    error
	}{
		{
			name:   "super admin role",
			in:     *testSuperAdminRole,
			out:    *testSuperAdminRole,
			result: true,
			err:    nil,
		},
		{
			name:   "network admin role",
			in:     *testNetworkAdminRole,
			out:    *testNetworkAdminRole,
			result: true,
			err:    nil,
		},
		{
			name:   "incorrect object type",
			in:     struct{ name string }{"testing"},
			out:    struct{ name string }{"testing"},
			result: true,
			err:    errInvalidInputType,
		},
	}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l

	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}
	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore: %v", err)
	}
	for _, test := range tests {
		ctx := context.TODO()
		txn := kvs.NewTxn()
		out, result, err := r.permissionTenantCheck(ctx, kvs, txn, "", apiintf.CreateOper, false, test.in)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected err [%v], got [%v]", test.name, test.err, err))
		Assert(t, result == test.result, fmt.Sprintf("[%s] test failed, expected result [%v], got [%v]", test.name, test.result, result))
		Assert(t, reflect.DeepEqual(out, test.out), fmt.Sprintf("[%s] test failed, expected obj [%v], got [%v]", test.name, test.out, out))
	}
}

func TestDeleteUserPref(t *testing.T) {
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

	tests := []struct {
		name     string
		oper     apiintf.APIOperType
		in       interface{}
		out      interface{}
		result   bool
		err      bool
		txnEmpty bool
	}{
		{
			name: "invalid request",
			oper: apiintf.CreateOper,
			in: auth.User{
				ObjectMeta: api.ObjectMeta{
					Name:   "hello",
					Tenant: "world",
				},
			},
			out: auth.User{
				ObjectMeta: api.ObjectMeta{
					Name:   "hello",
					Tenant: "world",
				},
			},
			result:   true,
			err:      false,
			txnEmpty: true,
		},
		{
			name: "valid request",
			oper: apiintf.DeleteOper,
			in: auth.User{
				ObjectMeta: api.ObjectMeta{
					Name:   "hello",
					Tenant: "world",
				},
			},
			out: auth.User{
				ObjectMeta: api.ObjectMeta{
					Name:   "hello",
					Tenant: "world",
				},
			},
			result:   true,
			err:      false,
			txnEmpty: false,
		},
	}
	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()
	for _, test := range tests {
		txn := kvs.NewTxn()

		out, ok, err := authHooks.deleteUserPref(ctx, kvs, txn, "", test.oper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, test.err == (err != nil), fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out), fmt.Sprintf("[%v] test failed, expected returned obj [%#v], got [%#v]", test.name, test.out, out))
		Assert(t, test.txnEmpty == txn.IsEmpty(), fmt.Sprintf("[%v] test failed, expected txn empty to be [%v], got [%v]", test.name, test.txnEmpty, txn.IsEmpty()))
	}
}

func TestGenerateUserPref(t *testing.T) {
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

	tests := []struct {
		name     string
		oper     apiintf.APIOperType
		in       interface{}
		out      interface{}
		result   bool
		err      bool
		txnEmpty bool
	}{
		{
			name: "valid request",
			oper: apiintf.CreateOper,
			in: auth.User{
				ObjectMeta: api.ObjectMeta{
					Name:   "hello",
					Tenant: "world",
				},
			},
			out: auth.User{
				ObjectMeta: api.ObjectMeta{
					Name:   "hello",
					Tenant: "world",
				},
			},
			result:   true,
			err:      false,
			txnEmpty: false,
		},
		{
			name: "invalid request",
			oper: apiintf.DeleteOper,
			in: auth.User{
				ObjectMeta: api.ObjectMeta{
					Name:   "hello",
					Tenant: "world",
				},
			},
			out: auth.User{
				ObjectMeta: api.ObjectMeta{
					Name:   "hello",
					Tenant: "world",
				},
			},
			result:   true,
			err:      false,
			txnEmpty: true,
		},
	}
	ctx, cancelFunc := context.WithTimeout(context.TODO(), 5*time.Second)
	defer cancelFunc()
	for _, test := range tests {
		txn := kvs.NewTxn()

		out, ok, err := authHooks.generateUserPref(ctx, kvs, txn, "", test.oper, false, test.in)
		Assert(t, test.result == ok, fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, test.err == (err != nil), fmt.Sprintf("[%v] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out), fmt.Sprintf("[%v] test failed, expected returned obj [%#v], got [%#v]", test.name, test.out, out))
		Assert(t, test.txnEmpty == txn.IsEmpty(), fmt.Sprintf("[%v] test failed, expected txn empty to be [%v], got [%v]", test.name, test.txnEmpty, txn.IsEmpty()))
	}
}
