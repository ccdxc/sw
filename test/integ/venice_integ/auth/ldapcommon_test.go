package auth

import (
	"fmt"
	"reflect"
	"sort"
	"testing"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn/ldap"

	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// authenticationPoliciesData returns ldap configs to test TLS and non TLS connections
func authenticationPoliciesData(config *LdapConfig) map[string]*auth.Ldap {
	ldapdata := make(map[string]*auth.Ldap)
	ldapdata["TLS Enabled"] = &auth.Ldap{
		Domains: []*auth.LdapDomain{
			{
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
				},
			},
		},
	}
	ldapdata["TLS Skip Server Verification"] = &auth.Ldap{
		Domains: []*auth.LdapDomain{
			{
				Servers: []*auth.LdapServer{
					{
						Url: config.URL,
						TLSOptions: &auth.TLSOptions{
							StartTLS:                   true,
							SkipServerCertVerification: true,
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
				},
			},
		},
	}
	ldapdata["Without TLS"] = &auth.Ldap{
		Domains: []*auth.LdapDomain{
			{
				Servers: []*auth.LdapServer{
					{
						Url: config.URL,
						TLSOptions: &auth.TLSOptions{
							StartTLS: false,
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
				},
			},
		},
	}

	return ldapdata
}

// createDefaultAuthenticationPolicy creates an authentication policy with LDAP with TLS enabled
func createDefaultAuthenticationPolicy(config *LdapConfig) *auth.AuthenticationPolicy {
	return MustCreateAuthenticationPolicy(tinfo.apicl,
		&auth.Local{}, &auth.Ldap{
			Domains: []*auth.LdapDomain{
				{
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
				},
			},
		},
		nil)
}

func testAuthenticate(t *testing.T, config *LdapConfig) {
	for testtype, ldapconf := range authenticationPoliciesData(config) {
		_, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{}, ldapconf)
		if err != nil {
			t.Errorf("err %s in CreateAuthenticationPolicy", err)
			return
		}
		// create password authenticator
		authenticator := ldap.NewLdapAuthenticator(ldapconf)

		// authenticate
		autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: config.LdapUser, Password: config.LdapUserPassword})
		DeleteAuthenticationPolicy(tinfo.apicl)

		Assert(t, ok, fmt.Sprintf("[%v] Unsuccessful ldap user authentication", testtype))
		Assert(t, autheduser.Name == config.LdapUser, fmt.Sprintf("[%v] User returned by ldap authenticator didn't match user being authenticated", testtype))
		Assert(t, autheduser.Spec.GetType() == auth.UserSpec_External.String(), fmt.Sprintf("[%v] User created is not of type External", testtype))
		userGroups := autheduser.Status.GetUserGroups()
		sort.Strings(userGroups)
		sort.Strings(config.LdapUserGroupsDN)
		Assert(t, reflect.DeepEqual(userGroups, config.LdapUserGroupsDN), fmt.Sprintf("[%v] Incorrect user group returned, expected [%v], got [%v]", testtype, config.LdapUserGroupsDN, userGroups))
		AssertOk(t, err, fmt.Sprintf("[%v] Error authenticating user", testtype))
	}
}

func testIncorrectPasswordAuthentication(t *testing.T, config *LdapConfig) {
	policy := createDefaultAuthenticationPolicy(config)
	defer DeleteAuthenticationPolicy(tinfo.apicl)

	// create ldap authenticator
	authenticator := ldap.NewLdapAuthenticator(policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: config.LdapUser, Password: "wrongpassword"})

	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned while authenticating with wrong password")
	Assert(t, err != nil, "No error returned while authenticating with wrong password")
}

func testIncorrectUserAuthentication(t *testing.T, config *LdapConfig) {
	policy := createDefaultAuthenticationPolicy(config)
	defer DeleteAuthenticationPolicy(tinfo.apicl)

	// create ldap authenticator
	authenticator := ldap.NewLdapAuthenticator(policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: "test1", Password: "password"})

	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned while authenticating with incorrect username")
	Assert(t, err != nil, "No error returned while authenticating with incorrect username")
	Assert(t, err == ldap.ErrNoneOrMultipleUserEntries, "Incorrect error type returned")
}

func testIncorrectLdapAttributeMapping(t *testing.T, config *LdapConfig) {
	policy, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{}, &auth.Ldap{
		Domains: []*auth.LdapDomain{
			{
				Servers: []*auth.LdapServer{
					{
						Url: tinfo.ldapAddr,
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
					User:             "cn",
					UserObjectClass:  "organization",
					Group:            "ou",
					GroupObjectClass: "groupOfNames",
				},
			},
		},
	})
	if err != nil {
		t.Errorf("err %s in CreateAuthenticationPolicy", err)
		return
	}
	defer DeleteAuthenticationPolicy(tinfo.apicl)

	// create ldap authenticator
	authenticator := ldap.NewLdapAuthenticator(policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: config.LdapUser, Password: config.LdapUserPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with misconfigured authentication policy: Incorrect LDAP Attribute Mapping")
	Assert(t, err != nil, "No error returned while authenticating with misconfigured authentication policy: Incorrect LDAP Attribute Mapping")
}

func testIncorrectBaseDN(t *testing.T, config *LdapConfig) {
	policy, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{}, &auth.Ldap{
		Domains: []*auth.LdapDomain{
			{
				Servers: []*auth.LdapServer{
					{
						Url: tinfo.ldapAddr,
						TLSOptions: &auth.TLSOptions{
							StartTLS:                   true,
							SkipServerCertVerification: false,
							ServerName:                 config.ServerName,
							TrustedCerts:               config.TrustedCerts,
						},
					},
				},
				BaseDN:       "DC=pensandoo,DC=io",
				BindDN:       config.BindDN,
				BindPassword: config.BindPassword,
				AttributeMapping: &auth.LdapAttributeMapping{
					User:             config.UserAttribute,
					UserObjectClass:  config.UserObjectClassAttribute,
					Group:            config.GroupAttribute,
					GroupObjectClass: config.GroupObjectClassAttribute,
				},
			},
		},
	})
	if err != nil {
		t.Errorf("err %s in CreateAuthenticationPolicy", err)
		return
	}
	defer DeleteAuthenticationPolicy(tinfo.apicl)

	// create ldap authenticator
	authenticator := ldap.NewLdapAuthenticator(policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: config.LdapUser, Password: config.LdapUserPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with misconfigured authentication policy: Incorrect Base DN")
	Assert(t, err != nil, "No error returned while authenticating with misconfigured authentication policy: Incorrect Base DN")
}

func testIncorrectBindPassword(t *testing.T, config *LdapConfig) {
	policy, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{}, &auth.Ldap{
		Domains: []*auth.LdapDomain{
			{
				Servers: []*auth.LdapServer{
					{
						Url: tinfo.ldapAddr,
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
				BindPassword: "wrongbindpassword",
				AttributeMapping: &auth.LdapAttributeMapping{
					User:             config.UserAttribute,
					UserObjectClass:  config.UserObjectClassAttribute,
					Group:            config.GroupAttribute,
					GroupObjectClass: config.GroupObjectClassAttribute,
				},
			},
		},
	})
	if err != nil {
		t.Errorf("err %s in CreateAuthenticationPolicy", err)
		return
	}
	defer DeleteAuthenticationPolicy(tinfo.apicl)

	// create ldap authenticator
	authenticator := ldap.NewLdapAuthenticator(policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: config.LdapUser, Password: config.LdapUserPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with misconfigured authentication policy: Incorrect Bind Password")
	Assert(t, err != nil, "No error returned while authenticating with misconfigured authentication policy: Incorrect Bind Password")
}

func testDisabledLdapAuthenticator(t *testing.T, config *LdapConfig) {
	policy, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{}, &auth.Ldap{
		Domains: []*auth.LdapDomain{
			{
				Servers: []*auth.LdapServer{
					{
						Url: tinfo.ldapAddr,
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
				},
			},
		},
	})

	if err != nil {
		t.Errorf("err %s in CreateAuthenticationPolicy", err)
		return
	}
	defer DeleteAuthenticationPolicy(tinfo.apicl)

	// create ldap authenticator
	authenticator := ldap.NewLdapAuthenticator(policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: config.LdapUser, Password: config.LdapUserPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with disabled LDAP authenticator")
	AssertOk(t, err, "Error returned with disabled LDAP authenticator")
}
