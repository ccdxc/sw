package manager

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestGetUser(t *testing.T) {
	authGetter := GetAuthGetter("AuthGetterTest", apiSrvAddr, nil, logger)
	AssertEventually(t, func() (bool, interface{}) {
		user, ok := authGetter.GetUser(testUser, tenant)
		return ok && user.Name == testUser && user.Tenant == tenant, nil
	}, fmt.Sprintf("[%v] user not found", testUser))
}

func TestStopStart(t *testing.T) {
	authGetter := GetAuthGetter("AuthGetterTest", apiSrvAddr, nil, logger)
	MustCreateTestUser(apicl, "test1", testPassword, tenant)
	defer DeleteUser(apicl, "test1", tenant)
	AssertEventually(t, func() (bool, interface{}) {
		user, ok := authGetter.GetUser("test1", tenant)
		return ok && user.Name == "test1" && user.Tenant == tenant, nil
	}, fmt.Sprintf("[%v] user not found", "test1"))
	// stop AuthGetter
	authGetter.Stop()
	// create user "test2"
	MustCreateTestUser(apicl, "test2", testPassword, tenant)
	defer DeleteUser(apicl, "test2", tenant)
	// check AuthGetter should still get "test2"
	AssertEventually(t, func() (bool, interface{}) {
		user, ok := authGetter.GetUser("test2", tenant)
		return ok && user.Name == "test2" && user.Tenant == tenant, nil
	}, fmt.Sprintf("[%v] user not found", "test2"))
	// start the watcher again
	authGetter.Start()
	AssertEventually(t, func() (bool, interface{}) {
		user, ok := authGetter.GetUser("test2", tenant)
		return ok && user.Name == "test2" && user.Tenant == tenant, nil
	}, fmt.Sprintf("[%v] user not found", "test2"))
}

func TestGetAuthenticators(t *testing.T) {
	var authenticators []authn.Authenticator
	var err error
	authGetter := GetAuthGetter("AuthGetterTest", apiSrvAddr, nil, logger)
	authenticators, err = authGetter.GetAuthenticators()
	Assert(t, err != nil && authenticators == nil, "authenticators created with no authentication policy defined")

	policy := &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Domains: []*auth.LdapDomain{
						{
							Servers: []*auth.LdapServer{
								{
									Url: "localhost:389",
									TLSOptions: &auth.TLSOptions{
										StartTLS:                   true,
										SkipServerCertVerification: false,
										ServerName:                 ServerName,
										TrustedCerts:               TrustedCerts,
									},
								},
							},

							BaseDN:       BaseDN,
							BindDN:       BindDN,
							BindPassword: BindPassword,
							AttributeMapping: &auth.LdapAttributeMapping{
								User:             UserAttribute,
								UserObjectClass:  UserObjectClassAttribute,
								Group:            GroupAttribute,
								GroupObjectClass: GroupObjectClassAttribute,
							},
						},
					},
				},
				Local:              &auth.Local{},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
			TokenExpiry: expiration.String(),
		},
	}
	createAuthenticationPolicy(t, policy)
	defer deleteAuthenticationPolicy(t)
	AssertEventually(t, func() (bool, interface{}) {
		authenticators, err = authGetter.GetAuthenticators()
		return err == nil && len(authenticators) == 2, nil
	}, "authenticators couldn't be created")
}

func TestGetTokenManager(t *testing.T) {
	var tokenMgr TokenManager
	var err error
	authGetter := GetAuthGetter("AuthGetterTest", apiSrvAddr, nil, logger)
	tokenMgr, err = authGetter.GetTokenManager()
	Assert(t, err != nil && tokenMgr == nil, "TokenManager created with no authentication policy defined")
	policy := &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Domains: []*auth.LdapDomain{
						{
							Servers: []*auth.LdapServer{
								{
									Url: "localhost:389",
									TLSOptions: &auth.TLSOptions{
										StartTLS:                   true,
										SkipServerCertVerification: false,
										ServerName:                 ServerName,
										TrustedCerts:               TrustedCerts,
									},
								},
							},

							BaseDN:       BaseDN,
							BindDN:       BindDN,
							BindPassword: BindPassword,
							AttributeMapping: &auth.LdapAttributeMapping{
								User:             UserAttribute,
								UserObjectClass:  UserObjectClassAttribute,
								Group:            GroupAttribute,
								GroupObjectClass: GroupObjectClassAttribute,
							},
						},
					},
				},
				Local:              &auth.Local{},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
			TokenExpiry: expiration.String(),
		},
	}
	createAuthenticationPolicy(t, policy)
	defer deleteAuthenticationPolicy(t)

	AssertEventually(t, func() (bool, interface{}) {
		tokenMgr, err = authGetter.GetTokenManager()
		return err == nil && tokenMgr != nil, nil
	}, "TokenManager couldn't be created")
}

func TestIsAuthBootstrapped(t *testing.T) {
	authGetter := GetAuthGetter("AuthGetterTest", apiSrvAddr, nil, logger)
	AssertEventually(t, func() (bool, interface{}) {
		ok, err := authGetter.IsAuthBootstrapped()
		return err == nil && !ok, err
	}, "cluster object should be available")
	MustSetAuthBootstrapFlag(apicl)
	AssertEventually(t, func() (bool, interface{}) {
		ok, err := authGetter.IsAuthBootstrapped()
		return err == nil && ok, err
	}, "auth bootstrap flag should be set")
}
