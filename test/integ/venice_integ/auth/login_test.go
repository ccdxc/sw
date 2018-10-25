package auth

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"

	. "github.com/pensando/sw/test/utils"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestLogin(t *testing.T) {
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	var resp *http.Response
	var statusCode int
	AssertEventually(t, func() (bool, interface{}) {
		var err error
		resp, err = Login(fmt.Sprintf("http://%s", tinfo.apiGwAddr), userCred)
		if err == nil {
			statusCode = resp.StatusCode
		}
		return err == nil && statusCode == http.StatusOK, err
	}, fmt.Sprintf("login request failed, Status code: %d", statusCode))

	// verify cookie
	cookies := resp.Cookies()
	Assert(t, len(cookies) == 1, "cookie not set in response")
	Assert(t, cookies[0].Name == "sid", "cookie not present")
	Assert(t, cookies[0].HttpOnly, "cookie is not http only")
	//Assert(t, cookies[0].Secure, "cookie is not secure") TODO:// Enable it once APIGW is TLS enabled
	Assert(t, cookies[0].MaxAge == apigw.TokenExpInDays*24*60*60, fmt.Sprintf("cookie max age is not 6 days, [%d]", cookies[0].MaxAge))
	Assert(t, cookies[0].Value != "", "session id value is not set")
	// verify user from response
	var user auth.User
	AssertOk(t, json.NewDecoder(resp.Body).Decode(&user), "unable to decode user from http response")
	Assert(t, user.Name == userCred.Username && user.Tenant == userCred.Tenant, fmt.Sprintf("incorrect user [%s] and tenant [%s] returned", user.Name, user.Tenant))
	Assert(t, user.Spec.Password == "", "password should be empty")
	Assert(t, len(user.Status.Roles) == 1 && user.Status.Roles[0] == globals.AdminRole, "user should have admin role")
	logintime, err := user.Status.LastSuccessfulLogin.Time()
	AssertOk(t, err, "error getting successful login time")
	Assert(t, time.Now().Sub(logintime) < time.Second, fmt.Sprintf("unexpected successful login time [%v]", logintime.Local()))
	Assert(t, user.Status.Authenticators[0] == auth.Authenticators_LOCAL.String(),
		fmt.Sprintf("expected authenticator [%s], got [%s]", auth.Authenticators_LOCAL.String(), user.Status.Authenticators[0]))
	// check CSRF token header is present
	Assert(t, resp.Header.Get(apigw.GrpcMDCsrfHeader) != "", "CSRF token not present")
}

func TestLoginFailures(t *testing.T) {
	tests := []struct {
		name     string
		cred     *auth.PasswordCredential
		expected int
	}{
		{
			name:     "non existent username",
			cred:     &auth.PasswordCredential{Username: "xxx", Password: "", Tenant: testTenant},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "invalid password",
			cred:     &auth.PasswordCredential{Username: testUser, Password: "xxx", Tenant: testTenant},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "invalid tenant",
			cred:     &auth.PasswordCredential{Username: testUser, Password: testPassword, Tenant: "xxx"},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "empty username",
			cred:     &auth.PasswordCredential{Username: "", Password: testPassword, Tenant: testTenant},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "empty username and password",
			cred:     &auth.PasswordCredential{Username: "", Password: "", Tenant: testTenant},
			expected: http.StatusUnauthorized,
		},
		{
			name:     "empty tenant",
			cred:     &auth.PasswordCredential{Username: testUser, Password: testPassword, Tenant: ""},
			expected: http.StatusUnauthorized,
		},
	}

	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	for _, test := range tests {
		var resp *http.Response
		var statusCode int
		AssertEventually(t, func() (bool, interface{}) {
			var err error
			resp, err = Login(fmt.Sprintf("http://%s", tinfo.apiGwAddr), test.cred)
			if err == nil {
				statusCode = resp.StatusCode
			}
			return err == nil && statusCode == test.expected, err
		}, fmt.Sprintf("[%v] test failed, returned status code [%d], expected [%d]", test.name, statusCode, test.expected))

		cookies := resp.Cookies()
		Assert(t, len(cookies) == 0, fmt.Sprintf("[%v] test failed, cookie should not be set in response", test.name))
		Assert(t, resp.Header.Get(apigw.GrpcMDCsrfHeader) == "", fmt.Sprintf("[%v] test failed, CSRF token is present", test.name))
	}
}

func TestUserPasswordRemoval(t *testing.T) {
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	restcl, err := apiclient.NewRestAPIClient(tinfo.apiGwAddr)
	if err != nil {
		panic("error creating rest client")
	}

	ctx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, userCred)
	AssertOk(t, err, "unable to get logged in context")
	// test GET user
	var user *auth.User
	AssertEventually(t, func() (bool, interface{}) {
		user, err = restcl.AuthV1().User().Get(ctx, &api.ObjectMeta{Name: testUser, Tenant: testTenant})
		return err == nil, nil
	}, "unable to fetch user")
	Assert(t, user.Spec.Password == "", fmt.Sprintf("Password should be removed from User object, %s", user.Spec.Password))
	// test LIST user
	var users []*auth.User
	AssertEventually(t, func() (bool, interface{}) {
		users, err = restcl.AuthV1().User().List(ctx, &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: testTenant}})
		return err == nil, nil
	}, "unable to fetch users")
	Assert(t, users[0].Spec.Password == "", fmt.Sprintf("Password should be removed from User object, %s", users[0].Spec.Password))
	// test UPDATE user
	AssertEventually(t, func() (bool, interface{}) {
		user, err = restcl.AuthV1().User().Update(ctx, &auth.User{
			TypeMeta: api.TypeMeta{Kind: "User"},
			ObjectMeta: api.ObjectMeta{
				Tenant: testTenant,
				Name:   testUser,
			},
			Spec: auth.UserSpec{
				Fullname: "Test User Update",
				Email:    "testuser@pensandio.io",
				Type:     auth.UserSpec_LOCAL.String(),
				Password: testPassword,
			},
		})
		return err == nil, nil
	}, "unable to update user")
	Assert(t, user.Spec.Password == "", fmt.Sprintf("Password should be removed from User object, %s", user.Spec.Password))
	// test CREATE user
	AssertEventually(t, func() (bool, interface{}) {
		user, err = restcl.AuthV1().User().Create(ctx, &auth.User{
			TypeMeta: api.TypeMeta{Kind: "User"},
			ObjectMeta: api.ObjectMeta{
				Tenant: testTenant,
				Name:   "testUser2",
			},
			Spec: auth.UserSpec{
				Fullname: "Test User2",
				Email:    "testuser2@pensandio.io",
				Type:     auth.UserSpec_LOCAL.String(),
				Password: testPassword,
			},
		})
		return err == nil, nil
	}, "unable to create user")
	Assert(t, user.Spec.Password == "", fmt.Sprintf("Password should be removed from User object, %s", user.Spec.Password))
	// test DELETE user
	AssertEventually(t, func() (bool, interface{}) {
		user, err = restcl.AuthV1().User().Delete(ctx, &api.ObjectMeta{Name: "testUser2", Tenant: testTenant})
		return err == nil, nil
	}, "unable to delete user")
	Assert(t, user.Spec.Password == "", fmt.Sprintf("Password should be removed from User object, %s", user.Spec.Password))
}

func TestAuthPolicy(t *testing.T) {
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	restcl, err := apiclient.NewRestAPIClient(tinfo.apiGwAddr)
	if err != nil {
		panic("error creating rest client")
	}

	ctx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, userCred)
	AssertOk(t, err, "unable to get logged in context")
	// test GET AuthenticationPolicy
	var policy *auth.AuthenticationPolicy
	AssertEventually(t, func() (bool, interface{}) {
		policy, err = restcl.AuthV1().AuthenticationPolicy().Get(ctx, &api.ObjectMeta{})
		if err != nil {
			log.Errorf("unable to fetch auth policy, Err: %v", err)
		}
		return err == nil, nil
	}, "unable to fetch auth policy")
	Assert(t, policy.Spec.Secret == nil, fmt.Sprintf("Secret [%#v] should be removed from AuthenticationPolicy object", policy.Spec.Secret))
	// test CREATE AuthenticationPolicy
	AssertConsistently(t, func() (bool, interface{}) {
		policy, err = restcl.AuthV1().AuthenticationPolicy().Create(ctx, &auth.AuthenticationPolicy{
			TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
			ObjectMeta: api.ObjectMeta{
				Name: "AuthenticationPolicy2",
			},
			Spec: auth.AuthenticationPolicySpec{
				Authenticators: auth.Authenticators{
					Ldap:               &auth.Ldap{Enabled: false},
					Local:              &auth.Local{Enabled: true},
					AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_LDAP.String()},
				},
			},
		})
		return err != nil, nil

	}, "cannot create more than one auth policy")
	// test UPDATE AuthenticationPolicy
	AssertEventually(t, func() (bool, interface{}) {
		policy, err = restcl.AuthV1().AuthenticationPolicy().Update(ctx, &auth.AuthenticationPolicy{
			TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
			ObjectMeta: api.ObjectMeta{
				Name: "AuthenticationPolicy3",
			},
			Spec: auth.AuthenticationPolicySpec{
				Authenticators: auth.Authenticators{
					Ldap:               &auth.Ldap{Enabled: false},
					Local:              &auth.Local{Enabled: true},
					AuthenticatorOrder: []string{auth.Authenticators_LOCAL.String(), auth.Authenticators_LDAP.String()},
				},
			},
		})
		return err == nil, nil
	}, "unable to update auth policy")
	Assert(t, policy.Name == "AuthenticationPolicy3", fmt.Sprintf("invalid auth policy name, [%s]", policy.Name))
	Assert(t, policy.Spec.Secret == nil, fmt.Sprintf("Secret [%#v] should be removed from AuthenticationPolicy object", policy.Spec.Secret))
	AssertEventually(t, func() (bool, interface{}) {
		policy, err = restcl.AuthV1().AuthenticationPolicy().Get(ctx, &api.ObjectMeta{})
		return err != nil, nil
	}, "User should need to login again after updating authentication policy")
	// re-login as JWT secret is regenerated after update to authentication policy
	ctx, err = NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, userCred)
	AssertOk(t, err, "unable to get logged in context")
	AssertEventually(t, func() (bool, interface{}) {
		policy, err = restcl.AuthV1().AuthenticationPolicy().Get(ctx, &api.ObjectMeta{})
		return err == nil, nil
	}, "unable to fetch auth policy")
	Assert(t, policy.Name == "AuthenticationPolicy3", "invalid auth policy name")
	Assert(t, policy.Spec.Secret == nil, fmt.Sprintf("Secret [%#v] should be removed from AuthenticationPolicy object", policy.Spec.Secret))
	// test DELETE AuthenticationPolicy
	AssertConsistently(t, func() (bool, interface{}) {
		policy, err = restcl.AuthV1().AuthenticationPolicy().Delete(ctx, &api.ObjectMeta{Name: "AuthenticationPolicy3"})
		return err != nil, nil
	}, "AuthenticationPolicy can't be deleted")
}

func TestUserStatus(t *testing.T) {
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, &auth.Ldap{Enabled: false}, &auth.Radius{Enabled: false}, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)

	currtime := time.Now()
	restcl, err := apiclient.NewRestAPIClient(tinfo.apiGwAddr)
	if err != nil {
		panic("error creating rest client")
	}
	ctx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, userCred)
	AssertOk(t, err, "unable to get logged in context")
	// test GET user
	var user *auth.User
	AssertEventually(t, func() (bool, interface{}) {
		user, err = restcl.AuthV1().User().Get(ctx, &api.ObjectMeta{Name: testUser, Tenant: testTenant})
		return err == nil, nil
	}, "unable to fetch user")
	Assert(t, len(user.Status.Roles) == 1 && user.Status.Roles[0] == globals.AdminRole, "user should have admin role")
	logintime, err := user.Status.LastSuccessfulLogin.Time()
	AssertOk(t, err, "error getting successful login time")
	Assert(t, logintime.After(currtime), fmt.Sprintf("login time [%v] not after current time [%v]", logintime.Local(), currtime.Local()))
	Assert(t, logintime.Sub(currtime) < 30*time.Second, fmt.Sprintf("login time [%v] not within 30 seconds of current time [%v]", logintime, currtime))
	Assert(t, user.Status.Authenticators[0] == auth.Authenticators_LOCAL.String(),
		fmt.Sprintf("expected authenticator [%s], got [%s]", auth.Authenticators_LOCAL.String(), user.Status.Authenticators[0]))
	// test LIST user
	var users []*auth.User
	AssertEventually(t, func() (bool, interface{}) {
		users, err = restcl.AuthV1().User().List(ctx, &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: testTenant}})
		return err == nil, nil
	}, "unable to fetch users")
	Assert(t, len(users[0].Status.Roles) == 1 && users[0].Status.Roles[0] == globals.AdminRole, "user should have admin role")
	MustCreateRole(tinfo.apicl, "NetworkAdminRole", testTenant, login.NewPermission(
		testTenant,
		"network",
		auth.Permission_Network.String(),
		authz.ResourceNamespaceAll,
		"",
		auth.Permission_ALL_ACTIONS.String()),
		login.NewPermission(
			testTenant,
			"auth",
			auth.Permission_AllResourceKinds.String(),
			authz.ResourceNamespaceAll,
			"",
			auth.Permission_ALL_ACTIONS.String()))
	defer MustDeleteRole(tinfo.apicl, "NetworkAdminRole", testTenant)
	MustCreateRoleBinding(tinfo.apicl, "NetworkAdminRoleBinding", testTenant, "NetworkAdminRole", []string{testUser}, nil)
	defer MustDeleteRoleBinding(tinfo.apicl, "NetworkAdminRoleBinding", testTenant)
	// login again to see if user status is updated with new login time
	ctx, err = NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, userCred)
	AssertOk(t, err, "unable to get logged in context")
	AssertEventually(t, func() (bool, interface{}) {
		user, err = restcl.AuthV1().User().Get(ctx, &api.ObjectMeta{Name: testUser, Tenant: testTenant})
		return err == nil, nil
	}, "unable to fetch user")
	Assert(t, len(user.Status.Roles) == 2, "user should have two roles")
	newlogintime, err := user.Status.LastSuccessfulLogin.Time()
	AssertOk(t, err, "error getting successful login time")
	Assert(t, newlogintime.After(logintime), fmt.Sprintf("new login time [%v] not after old login time [%v]", newlogintime.Local(), logintime.Local()))
}

func TestLdapLogin(t *testing.T) {
	localUserCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	config := getADConfig()
	ldapConf := &auth.Ldap{
		Enabled: true,
		Servers: []*auth.LdapServer{
			{
				Url: config.URL,
				TLSOptions: &auth.TLSOptions{
					StartTLS:                   true,
					SkipServerCertVerification: false,
					ServerName:                 config.ServerName,
					TrustedCerts:               config.TrustedCerts,
				},
			},
		},

		BaseDN:       config.BaseDN,
		BindDN:       config.BindDN,
		BindPassword: config.BindPassword,
		AttributeMapping: &auth.LdapAttributeMapping{
			User:             config.UserAttribute,
			UserObjectClass:  config.UserObjectClassAttribute,
			Group:            config.GroupAttribute,
			GroupObjectClass: config.GroupObjectClassAttribute,
			Tenant:           config.TenantAttribute,
		},
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, ldapConf, &auth.Radius{Enabled: false}, localUserCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, true, localUserCred, tinfo.l)
	ldapUserCred := &auth.PasswordCredential{
		Username: config.LdapUser,
		Password: config.LdapUserPassword,
	}

	currtime := time.Now()
	restcl, err := apiclient.NewRestAPIClient(tinfo.apiGwAddr)
	if err != nil {
		panic("error creating rest client")
	}
	MustCreateRoleBinding(tinfo.apicl, "LdapAdminRoleBinding", testTenant, globals.AdminRole, nil, config.LdapUserGroupsDN)
	defer MustDeleteRoleBinding(tinfo.apicl, "LdapAdminRoleBinding", testTenant)
	ctx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, ldapUserCred)
	AssertOk(t, err, "unable to get logged in context")
	// test GET user
	var user *auth.User
	AssertEventually(t, func() (bool, interface{}) {
		user, err = restcl.AuthV1().User().Get(ctx, &api.ObjectMeta{Name: config.LdapUser, Tenant: testTenant})
		return err == nil, nil
	}, "unable to fetch user")
	defer MustDeleteUser(tinfo.apicl, config.LdapUser, testTenant)
	Assert(t, user.Spec.Type == auth.UserSpec_EXTERNAL.String(), "unexpected user type: %s", user.Spec.Type)
	Assert(t, len(user.Status.Roles) == 1 && user.Status.Roles[0] == globals.AdminRole, "user should have admin role")
	logintime, err := user.Status.LastSuccessfulLogin.Time()
	AssertOk(t, err, "error getting successful login time")
	Assert(t, logintime.After(currtime), fmt.Sprintf("login time [%v] not after current time [%v]", logintime.Local(), currtime.Local()))
	Assert(t, logintime.Sub(currtime) < 30*time.Second, fmt.Sprintf("login time [%v] not within 30 seconds of current time [%v]", logintime, currtime))
	Assert(t, user.Status.Authenticators[0] == auth.Authenticators_LDAP.String(),
		fmt.Sprintf("expected authenticator [%s], got [%s]", auth.Authenticators_LDAP.String(), user.Status.Authenticators[0]))
}

func TestUsernameConflict(t *testing.T) {
	localUserCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   testTenant,
	}
	config := getADConfig()
	ldapConf := &auth.Ldap{
		Enabled: true,
		Servers: []*auth.LdapServer{
			{
				Url: config.URL,
				TLSOptions: &auth.TLSOptions{
					StartTLS:                   true,
					SkipServerCertVerification: false,
					ServerName:                 config.ServerName,
					TrustedCerts:               config.TrustedCerts,
				},
			},
		},

		BaseDN:       config.BaseDN,
		BindDN:       config.BindDN,
		BindPassword: config.BindPassword,
		AttributeMapping: &auth.LdapAttributeMapping{
			User:             config.UserAttribute,
			UserObjectClass:  config.UserObjectClassAttribute,
			Group:            config.GroupAttribute,
			GroupObjectClass: config.GroupObjectClassAttribute,
			Tenant:           config.TenantAttribute,
		},
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, ldapConf, &auth.Radius{Enabled: false}, localUserCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, true, localUserCred, tinfo.l)
	ldapUserCred := &auth.PasswordCredential{
		Username: config.LdapUser,
		Password: config.LdapUserPassword,
	}
	// create local user with same name as ldap user
	MustCreateTestUser(tinfo.apicl, config.LdapUser, testPassword, testTenant)
	var resp *http.Response
	var statusCode int
	AssertEventually(t, func() (bool, interface{}) {
		var err error
		resp, err = Login(fmt.Sprintf("http://%s", tinfo.apiGwAddr), ldapUserCred)
		if err == nil {
			statusCode = resp.StatusCode
		}
		return err == nil && statusCode == http.StatusConflict, err
	}, fmt.Sprintf("for username conflict expected status code [%d], got [%d]", http.StatusConflict, statusCode))
	MustDeleteUser(tinfo.apicl, config.LdapUser, testTenant)
	// ldap login should succeed after local user is deleted
	AssertEventually(t, func() (bool, interface{}) {
		var err error
		resp, err = Login(fmt.Sprintf("http://%s", tinfo.apiGwAddr), ldapUserCred)
		if err == nil {
			statusCode = resp.StatusCode
		}
		return err == nil && statusCode == http.StatusOK, err
	}, fmt.Sprintf("login request after deletion of local user with same name should succeed, Status code: %d", statusCode))
	defer MustDeleteUser(tinfo.apicl, config.LdapUser, testTenant)
	var user auth.User
	AssertOk(t, json.NewDecoder(resp.Body).Decode(&user), "unable to decode user from http response")
	Assert(t, user.Status.Authenticators[0] == auth.Authenticators_LDAP.String(),
		fmt.Sprintf("expected authenticator [%s], got [%s]", auth.Authenticators_LDAP.String(), user.Status.Authenticators[0]))
	Assert(t, user.Spec.Type == auth.UserSpec_EXTERNAL.String(), fmt.Sprintf("expected external user type, got [%s]", user.Spec.Type))
}

func TestLdapChecks(t *testing.T) {
	config := getADConfig()
	ldapConf := &auth.Ldap{
		Enabled: true,
		Servers: []*auth.LdapServer{
			{
				Url: config.URL,
				TLSOptions: &auth.TLSOptions{
					StartTLS:                   true,
					SkipServerCertVerification: false,
					ServerName:                 config.ServerName,
					TrustedCerts:               config.TrustedCerts,
				},
			},
		},

		BaseDN:       config.BaseDN,
		BindDN:       config.BindDN,
		BindPassword: config.BindPassword,
		AttributeMapping: &auth.LdapAttributeMapping{
			User:             config.UserAttribute,
			UserObjectClass:  config.UserObjectClassAttribute,
			Group:            config.GroupAttribute,
			GroupObjectClass: config.GroupObjectClassAttribute,
			Tenant:           config.TenantAttribute,
		},
	}
	MustCreateCluster(tinfo.apicl)
	defer MustDeleteCluster(tinfo.apicl)
	// create authentication policy
	MustCreateAuthenticationPolicy(tinfo.apicl, &auth.Local{Enabled: true}, ldapConf, &auth.Radius{Enabled: false})
	defer MustDeleteAuthenticationPolicy(tinfo.apicl)
	restcl, err := apiclient.NewRestAPIClient(tinfo.apiGwAddr)
	if err != nil {
		panic("error creating rest client")
	}
	var policy *auth.AuthenticationPolicy
	AssertEventually(t, func() (bool, interface{}) {
		policy, err = restcl.AuthV1().AuthenticationPolicy().Get(context.TODO(), &api.ObjectMeta{})
		if err != nil {
			log.Errorf("unable to fetch auth policy, Err: %v", err)
		}
		return err == nil, nil
	}, "unable to fetch auth policy")
	// connection check
	var retpolicy *auth.AuthenticationPolicy
	AssertEventually(t, func() (bool, interface{}) {
		retpolicy, err = restcl.AuthV1().AuthenticationPolicy().LdapConnectionCheck(context.TODO(), policy)
		if err != nil {
			log.Errorf("unable to check ldap connection, Err: %v", err)
		}
		return err == nil, nil
	}, "unable to check ldap connection")
	Assert(t, len(retpolicy.Status.LdapServers) == 1, fmt.Sprintf("unexpected number of ldap servers status: %d", len(retpolicy.Status.LdapServers)))
	Assert(t, retpolicy.Status.LdapServers[0].Result == auth.LdapServerStatus_Connect_Success.String(), "expected ldap connection check to succeed")
	Assert(t, retpolicy.Status.LdapServers[0].Server.Url == policy.Spec.Authenticators.Ldap.Servers[0].Url,
		fmt.Sprintf("expected ldap url [%s], got [%s] in ldap server status", policy.Spec.Authenticators.Ldap.Servers[0].Url, retpolicy.Status.LdapServers[0].Server.Url))
	// bind check
	AssertEventually(t, func() (bool, interface{}) {
		retpolicy, err = restcl.AuthV1().AuthenticationPolicy().LdapBindCheck(context.TODO(), policy)
		if err != nil {
			log.Errorf("unable to check bind on ldap connection, Err: %v", err)
		}
		return err == nil, nil
	}, "unable to check bind on ldap connection")
	Assert(t, len(retpolicy.Status.LdapServers) == 1, fmt.Sprintf("unexpected number of ldap servers status: %d", len(retpolicy.Status.LdapServers)))
	Assert(t, retpolicy.Status.LdapServers[0].Result == auth.LdapServerStatus_Bind_Success.String(), "expected ldap bind check to succeed")
	Assert(t, retpolicy.Status.LdapServers[0].Server.Url == policy.Spec.Authenticators.Ldap.Servers[0].Url,
		fmt.Sprintf("expected ldap url [%s], got [%s] in ldap server status", policy.Spec.Authenticators.Ldap.Servers[0].Url, retpolicy.Status.LdapServers[0].Server.Url))
	// check incorrect bind password
	policy.Spec.Authenticators.Ldap.BindPassword = "incorrectpasswd"
	AssertEventually(t, func() (bool, interface{}) {
		retpolicy, err = restcl.AuthV1().AuthenticationPolicy().LdapBindCheck(context.TODO(), policy)
		if err != nil {
			log.Errorf("unable to check bind on ldap connection, Err: %v", err)
		}
		return err == nil, nil
	}, "unable to check bind on ldap connection")
	Assert(t, len(retpolicy.Status.LdapServers) == 1, fmt.Sprintf("unexpected number of ldap servers status: %d", len(retpolicy.Status.LdapServers)))
	Assert(t, retpolicy.Status.LdapServers[0].Result == auth.LdapServerStatus_Bind_Failure.String(), "expected ldap bind check to fail")
	Assert(t, retpolicy.Status.LdapServers[0].Server.Url == policy.Spec.Authenticators.Ldap.Servers[0].Url,
		fmt.Sprintf("expected ldap url [%s], got [%s] in ldap server status", policy.Spec.Authenticators.Ldap.Servers[0].Url, retpolicy.Status.LdapServers[0].Server.Url))
	policy.Spec.Authenticators.Ldap.BindPassword = config.BindPassword
	// check connection failure
	policy.Spec.Authenticators.Ldap.Servers[0].Url = "unknown:333"
	AssertEventually(t, func() (bool, interface{}) {
		retpolicy, err = restcl.AuthV1().AuthenticationPolicy().LdapConnectionCheck(context.TODO(), policy)
		if err != nil {
			log.Errorf("unable to check bind on ldap connection, Err: %v", err)
		}
		return err == nil, nil
	}, "unable to check bind on ldap connection")
	Assert(t, len(retpolicy.Status.LdapServers) == 1, fmt.Sprintf("unexpected number of ldap servers status: %d", len(retpolicy.Status.LdapServers)))
	Assert(t, retpolicy.Status.LdapServers[0].Result == auth.LdapServerStatus_Connect_Failure.String(), "expected ldap connection check to fail")
	Assert(t, retpolicy.Status.LdapServers[0].Server.Url == policy.Spec.Authenticators.Ldap.Servers[0].Url,
		fmt.Sprintf("expected ldap url [%s], got [%s] in ldap server status", policy.Spec.Authenticators.Ldap.Servers[0].Url, retpolicy.Status.LdapServers[0].Server.Url))
}

func TestRadiusLogin(t *testing.T) {
	config := getACSConfig()
	localUserCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   config.Tenant,
	}
	radiusConf := &auth.Radius{
		Enabled: true,
		Servers: []*auth.RadiusServer{{
			Url:        config.URL,
			Secret:     config.NasSecret,
			AuthMethod: auth.Radius_PAP.String(),
		}},
		NasID: config.NasID,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, &auth.Ldap{Enabled: false}, radiusConf, localUserCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, true, localUserCred, tinfo.l)
	radiusUserCred := &auth.PasswordCredential{
		Username: config.User,
		Password: config.Password,
	}

	currtime := time.Now()
	restcl, err := apiclient.NewRestAPIClient(tinfo.apiGwAddr)
	if err != nil {
		panic("error creating rest client")
	}
	MustCreateRoleBinding(tinfo.apicl, "RadiusAdminRoleBinding", config.Tenant, globals.AdminRole, nil, config.UserGroups)
	defer MustDeleteRoleBinding(tinfo.apicl, "RadiusAdminRoleBinding", config.Tenant)
	ctx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, radiusUserCred)
	AssertOk(t, err, "unable to get logged in context")
	// test GET user
	var user *auth.User
	AssertEventually(t, func() (bool, interface{}) {
		user, err = restcl.AuthV1().User().Get(ctx, &api.ObjectMeta{Name: config.User, Tenant: config.Tenant})
		return err == nil, nil
	}, "unable to fetch user")
	defer MustDeleteUser(tinfo.apicl, config.User, config.Tenant)
	Assert(t, user.Spec.Type == auth.UserSpec_EXTERNAL.String(), "unexpected user type: %s", user.Spec.Type)
	Assert(t, len(user.Status.Roles) == 1 && user.Status.Roles[0] == globals.AdminRole, "user should have admin role")
	logintime, err := user.Status.LastSuccessfulLogin.Time()
	AssertOk(t, err, "error getting successful login time")
	Assert(t, logintime.After(currtime), fmt.Sprintf("login time [%v] not after current time [%v]", logintime.Local(), currtime.Local()))
	Assert(t, logintime.Sub(currtime) < 30*time.Second, fmt.Sprintf("login time [%v] not within 30 seconds of current time [%v]", logintime, currtime))
	Assert(t, user.Status.Authenticators[0] == auth.Authenticators_RADIUS.String(),
		fmt.Sprintf("expected authenticator [%s], got [%s]", auth.Authenticators_RADIUS.String(), user.Status.Authenticators[0]))
}
