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
	authGetter := GetAuthGetter("AuthGetterTest", apiSrvAddr, nil, 600)
	AssertEventually(t, func() (bool, interface{}) {
		user, ok := authGetter.GetUser(testUser, tenant)
		return ok && user.Name == testUser && user.Tenant == tenant, nil
	}, fmt.Sprintf("[%v] user not found", testUser))
}

func TestStopStart(t *testing.T) {
	authGetter := GetAuthGetter("AuthGetterTest", apiSrvAddr, nil, 600)
	CreateTestUser(apicl, "test1", testPassword, tenant)
	AssertEventually(t, func() (bool, interface{}) {
		user, ok := authGetter.GetUser("test1", tenant)
		return ok && user.Name == "test1" && user.Tenant == tenant, nil
	}, fmt.Sprintf("[%v] user not found", "test1"))
	// stop AuthGetter
	authGetter.Stop()
	// create user "test2"
	CreateTestUser(apicl, "test2", testPassword, tenant)
	// check AuthGetter shouldn't get "test2"
	AssertConsistently(t, func() (bool, interface{}) {
		user, ok := authGetter.GetUser("test2", tenant)
		return !ok && user == nil, nil
	}, fmt.Sprintf("[%v] user found", "test2"))
	// start the watcher again
	authGetter.Start()
	AssertEventually(t, func() (bool, interface{}) {
		user, ok := authGetter.GetUser("test2", tenant)
		return ok && user.Name == "test2" && user.Tenant == tenant, nil
	}, fmt.Sprintf("[%v] user not found", "test2"))
	DeleteUser(apicl, "test1", tenant)
	DeleteUser(apicl, "test2", tenant)
}

func TestGetAuthenticators(t *testing.T) {
	var authenticators []authn.Authenticator
	var err error
	authGetter := GetAuthGetter("AuthGetterTest", apiSrvAddr, nil, 600)
	AssertConsistently(t, func() (bool, interface{}) {
		authenticators, err = authGetter.GetAuthenticators()
		return err != nil && authenticators == nil, nil
	}, "authenticators created with no authentication policy defined")

	policy := &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: true,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
			Secret: secret,
		},
	}
	createAuthenticationPolicy(policy)
	defer DeleteAuthenticationPolicy(apicl)
	AssertEventually(t, func() (bool, interface{}) {
		authenticators, err = authGetter.GetAuthenticators()
		return err == nil && len(authenticators) == 2, nil
	}, "authenticators couldn't be created")
}

func TestGetTokenManager(t *testing.T) {
	var tokenMgr TokenManager
	var err error
	authGetter := GetAuthGetter("AuthGetterTest", apiSrvAddr, nil, 600)
	AssertConsistently(t, func() (bool, interface{}) {
		tokenMgr, err = authGetter.GetTokenManager()
		return err != nil && tokenMgr == nil, nil
	}, "TokenManager created with no authentication policy defined")
	policy := &auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap: &auth.Ldap{
					Enabled: true,
				},
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
			},
			Secret: secret,
		},
	}
	createAuthenticationPolicy(policy)
	defer DeleteAuthenticationPolicy(apicl)

	AssertEventually(t, func() (bool, interface{}) {
		tokenMgr, err = authGetter.GetTokenManager()
		return err == nil && tokenMgr != nil, nil
	}, "TokenManager couldn't be created")
}
