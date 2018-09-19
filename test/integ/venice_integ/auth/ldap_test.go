package auth

import (
	"fmt"
	"os"
	"reflect"
	"sort"
	"testing"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn/ldap"
	"github.com/pensando/sw/venice/utils/log"

	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apiserver"
)

const (
	ldapServer           = "openldap"
	referralServer       = "openldapref"
	ldapUser             = "testuser"
	ldapUserGroupDN      = "cn=Administrators,dc=pensando,dc=io"
	ldapUserPassword     = "pensando"
	referralUser         = "testReferral"
	referralUserDN       = "cn=testReferral,dc=pensando,dc=io"
	referralUserGroupDN  = "cn=AdministratorsReferral,dc=pensando,dc=io"
	referralUserPassword = "pensando"
)

func setupLdap() {
	var err error
	// start ldap server
	tinfo.ldapAddr, err = StartLdapServer(ldapServer)
	if err != nil {
		log.Errorf("Error creating LDAP Server: %v", err)
		os.Exit(-1)
	}
	// start referral server
	tinfo.referralAddr, err = StartLdapServer(referralServer)
	if err != nil {
		StopLdapServer(ldapServer)
		log.Errorf("Error creating referral LDAP Server: %v", err)
		os.Exit(-1)
	}
	// create test ldap user
	err = CreateLdapUser(tinfo.ldapAddr, ldapUser, ldapUserPassword, testTenant, []string{ldapUserGroupDN})
	if err != nil {
		shutdownLdap()
		log.Errorf("Error creating test ldap user: %v", err)
		os.Exit(-1)
	}
	// create testReferral ldap user in referral server
	err = CreateLdapUser(tinfo.referralAddr, referralUser, referralUserPassword, testTenant, []string{referralUserGroupDN})
	if err != nil {
		shutdownLdap()
		log.Errorf("Error creating test ldap user: %v", err)
		os.Exit(-1)
	}
	// create testReferral ldap group in referral server
	err = CreateGroup(tinfo.referralAddr, referralUserGroupDN, []string{ldapUserGroupDN}, []string{referralUserDN})
	if err != nil {
		shutdownLdap()
		log.Errorf("Error creating testReferral ldap group in referral server: %v", err)
		os.Exit(-1)
	}
	// create referral entry in ldap server
	err = CreateReferral(tinfo.ldapAddr, referralUser, "ldap://"+tinfo.referralAddr+"/"+BaseDN)
	if err != nil {
		shutdownLdap()
		log.Errorf("Error creating testReferral referral entry in ldap server: %v", err)
		os.Exit(-1)
	}
}

func shutdownLdap() {
	StopLdapServer(ldapServer)
	StopLdapServer(referralServer)
}

// authenticationPoliciesData returns ldap configs to test TLS and non TLS connections
func authenticationPoliciesData() map[string]*auth.Ldap {
	ldapdata := make(map[string]*auth.Ldap)
	ldapdata["TLS Enabled"] = &auth.Ldap{
		Enabled: true,
		Servers: []*auth.LdapServer{
			{
				Url: tinfo.ldapAddr,
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
	}
	ldapdata["TLS Skip Server Verification"] = &auth.Ldap{
		Enabled: true,
		Servers: []*auth.LdapServer{
			{
				Url: tinfo.ldapAddr,
				TLSOptions: &auth.TLSOptions{
					StartTLS:                   true,
					SkipServerCertVerification: true,
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
	}
	ldapdata["Without TLS"] = &auth.Ldap{
		Enabled: true,
		Servers: []*auth.LdapServer{
			{
				Url: tinfo.ldapAddr,
				TLSOptions: &auth.TLSOptions{
					StartTLS: false,
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
	}

	return ldapdata
}

// createDefaultAuthenticationPolicy creates an authentication policy with LDAP with TLS enabled
func createDefaultAuthenticationPolicy() *auth.AuthenticationPolicy {
	return MustCreateAuthenticationPolicy(tinfo.apicl,
		&auth.Local{
			Enabled: true,
		}, &auth.Ldap{
			Enabled: true,
			Servers: []*auth.LdapServer{
				{
					Url: tinfo.ldapAddr,
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
				Tenant:           TenantAttribute,
			},
		})
}

func TestAuthenticate(t *testing.T) {
	t.Skip()
	for testtype, ldapconf := range authenticationPoliciesData() {
		_, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{Enabled: true}, ldapconf)
		if err != nil {
			t.Errorf("err %s in CreateAuthenticationPolicy", err)
			return
		}
		// create password authenticator
		authenticator := ldap.NewLdapAuthenticator(ldapconf)

		// authenticate
		autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: ldapUser, Password: ldapUserPassword})
		DeleteAuthenticationPolicy(tinfo.apicl)

		Assert(t, ok, fmt.Sprintf("[%v] Unsuccessful ldap user authentication", testtype))
		Assert(t, autheduser.Name == ldapUser, fmt.Sprintf("[%v] User returned by ldap authenticator didn't match user being authenticated", testtype))
		Assert(t, autheduser.Spec.GetType() == auth.UserSpec_EXTERNAL.String(), fmt.Sprintf("[%v] User created is not of type EXTERNAL", testtype))
		Assert(t, autheduser.Status.GetUserGroups()[0] == ldapUserGroupDN,
			fmt.Sprintf("[%v] Incorrect user group returned, expected [%s], got [%s]", testtype, ldapUserGroupDN, autheduser.Status.GetUserGroups()[0]))
		AssertOk(t, err, fmt.Sprintf("[%v] Error authenticating user", testtype))
	}

}

func TestIncorrectPasswordAuthentication(t *testing.T) {
	t.Skip()
	policy := createDefaultAuthenticationPolicy()
	defer DeleteAuthenticationPolicy(tinfo.apicl)

	// create ldap authenticator
	authenticator := ldap.NewLdapAuthenticator(policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: ldapUser, Password: "wrongpassword"})

	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned while authenticating with wrong password")
	Assert(t, err != nil, "No error returned while authenticating with wrong password")
}

func TestIncorrectUserAuthentication(t *testing.T) {
	t.Skip()
	policy := createDefaultAuthenticationPolicy()
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

func TestMissingLdapAttributeMapping(t *testing.T) {
	t.Skip()
	policy, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{Enabled: true}, &auth.Ldap{
		Enabled: true,
		Servers: []*auth.LdapServer{
			{
				Url: tinfo.ldapAddr,
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
	})
	if err != nil {
		t.Errorf("err %s in CreateAuthenticationPolicy", err)
		return
	}
	defer DeleteAuthenticationPolicy(tinfo.apicl)

	// create ldap authenticator
	authenticator := ldap.NewLdapAuthenticator(policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: ldapUser, Password: ldapUserPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with misconfigured authentication policy: Missing LDAP Attribute Mapping")
	Assert(t, err != nil, "No error returned while authenticating with misconfigured authentication policy: Missing LDAP Attribute Mapping")
}

func TestIncorrectLdapAttributeMapping(t *testing.T) {
	t.Skip()
	policy, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{Enabled: true}, &auth.Ldap{
		Enabled: true,
		Servers: []*auth.LdapServer{
			{
				Url: tinfo.ldapAddr,
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
			User:             "cn",
			UserObjectClass:  "organization",
			Group:            "ou",
			GroupObjectClass: "groupOfNames",
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
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: ldapUser, Password: ldapUserPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with misconfigured authentication policy: Incorrect LDAP Attribute Mapping")
	Assert(t, err != nil, "No error returned while authenticating with misconfigured authentication policy: Incorrect LDAP Attribute Mapping")
}

func TestIncorrectBaseDN(t *testing.T) {
	t.Skip()
	policy, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{Enabled: true}, &auth.Ldap{
		Enabled: true,
		Servers: []*auth.LdapServer{
			{
				Url: tinfo.ldapAddr,
				TLSOptions: &auth.TLSOptions{
					StartTLS:                   true,
					SkipServerCertVerification: false,
					ServerName:                 ServerName,
					TrustedCerts:               TrustedCerts,
				},
			},
		},
		BaseDN:       "DC=pensandoo,DC=io",
		BindDN:       BindDN,
		BindPassword: BindPassword,
		AttributeMapping: &auth.LdapAttributeMapping{
			User:             UserAttribute,
			UserObjectClass:  UserObjectClassAttribute,
			Group:            GroupAttribute,
			GroupObjectClass: GroupObjectClassAttribute,
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
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: ldapUser, Password: ldapUserPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with misconfigured authentication policy: Incorrect Base DN")
	Assert(t, err != nil, "No error returned while authenticating with misconfigured authentication policy: Incorrect Base DN")
}

func TestIncorrectBindPassword(t *testing.T) {
	t.Skip()
	policy, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{Enabled: true}, &auth.Ldap{
		Enabled: true,
		Servers: []*auth.LdapServer{
			{
				Url: tinfo.ldapAddr,
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
		BindPassword: "wrongbindpassword",
		AttributeMapping: &auth.LdapAttributeMapping{
			User:             UserAttribute,
			UserObjectClass:  UserObjectClassAttribute,
			Group:            GroupAttribute,
			GroupObjectClass: GroupObjectClassAttribute,
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
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: ldapUser, Password: ldapUserPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with misconfigured authentication policy: Incorrect Bind Password")
	Assert(t, err != nil, "No error returned while authenticating with misconfigured authentication policy: Incorrect Bind Password")
}

func TestDisabledLdapAuthenticator(t *testing.T) {
	t.Skip()
	policy, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{Enabled: true}, &auth.Ldap{
		Enabled: false,
		Servers: []*auth.LdapServer{
			{
				Url: tinfo.ldapAddr,
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
	})

	if err != nil {
		t.Errorf("err %s in CreateAuthenticationPolicy", err)
		return
	}
	defer DeleteAuthenticationPolicy(tinfo.apicl)

	// create ldap authenticator
	authenticator := ldap.NewLdapAuthenticator(policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: ldapUser, Password: ldapUserPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with disabled LDAP authenticator")
	AssertOk(t, err, "Error returned with disabled LDAP authenticator")
}

func TestReferral(t *testing.T) {
	t.Skip()
	for testtype, ldapconf := range authenticationPoliciesData() {
		_, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{Enabled: true}, ldapconf)
		if err != nil {
			t.Errorf("err %s in CreateAuthenticationPolicy", err)
			return
		}
		// create password authenticator
		authenticator := ldap.NewLdapAuthenticator(ldapconf)

		// authenticate
		autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: referralUser, Password: referralUserPassword})
		DeleteAuthenticationPolicy(tinfo.apicl)

		Assert(t, ok, fmt.Sprintf("[%v] Unsuccessful ldap user authentication", testtype))
		Assert(t, autheduser.Name == referralUser, fmt.Sprintf("[%v] User returned by ldap authenticator didn't match user being authenticated", testtype))
		Assert(t, autheduser.Spec.GetType() == auth.UserSpec_EXTERNAL.String(), fmt.Sprintf("[%v] User created is not of type EXTERNAL", testtype))
		returnedGroups := autheduser.Status.GetUserGroups()
		sort.Strings(returnedGroups)
		expectedGroups := []string{ldapUserGroupDN, referralUserGroupDN}
		sort.Strings(expectedGroups)
		Assert(t, reflect.DeepEqual(autheduser.Status.GetUserGroups(), expectedGroups),
			fmt.Sprintf("[%v] Incorrect user group returned, expected [%v], got [%v]", testtype, expectedGroups, returnedGroups))
		AssertOk(t, err, fmt.Sprintf("[%v] Error authenticating user", testtype))
	}
}
