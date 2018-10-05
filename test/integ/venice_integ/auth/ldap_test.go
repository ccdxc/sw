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
)

func getOpenLdapConfig() *LdapConfig {
	return &LdapConfig{
		ServerName: "0a7af420ff67",
		TrustedCerts: `-----BEGIN CERTIFICATE-----
MIIC/TCCAoOgAwIBAgIUF58P7j/wJUrJXKM1LVlrWRaAc8wwCgYIKoZIzj0EAwMw
gZYxCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxBMUEgQ2FyIFdhc2gxJDAiBgNVBAsT
G0luZm9ybWF0aW9uIFRlY2hub2xvZ3kgRGVwLjEUMBIGA1UEBxMLQWxidXF1ZXJx
dWUxEzARBgNVBAgTCk5ldyBNZXhpY28xHzAdBgNVBAMTFmRvY2tlci1saWdodC1i
YXNlaW1hZ2UwHhcNMTcxMjEzMjIyNDAwWhcNMTgxMjEzMjIyNDAwWjCBjDELMAkG
A1UEBhMCVVMxEzARBgNVBAgTCk5ldyBNZXhpY28xFDASBgNVBAcTC0FsYnVxdWVy
cXVlMRUwEwYDVQQKEwxBMUEgQ2FyIFdhc2gxJDAiBgNVBAsTG0luZm9ybWF0aW9u
IFRlY2hub2xvZ3kgRGVwLjEVMBMGA1UEAxMMMGE3YWY0MjBmZjY3MHYwEAYHKoZI
zj0CAQYFK4EEACIDYgAE0kmi9mFmxknKd5nDSTG/aIzTvJ3Uza7kNJzNP8+F9Fsb
F9A2N0uDcGuEYZfDwfwHcoUIw/+1kNy+endSrAipOYSEZN91bOdGAOzZE+JsrLhW
yS3MrFIUviI1qevigvJwo4GZMIGWMA4GA1UdDwEB/wQEAwIFoDAdBgNVHSUEFjAU
BggrBgEFBQcDAQYIKwYBBQUHAwIwDAYDVR0TAQH/BAIwADAdBgNVHQ4EFgQUOJFK
TNnQJCm5qhClodH6dCz3zkswHwYDVR0jBBgwFoAUT6XpeiVcMBicYtOXhbpQsoeu
bvgwFwYDVR0RBBAwDoIMMGE3YWY0MjBmZjY3MAoGCCqGSM49BAMDA2gAMGUCMQCC
wNQ0bAkWU27WgzOhn0m7wh87W2U9NX0xJLGNDFsjwfn26uagp46V4h2UKVICe98C
MFB3stnk7Lfr/w/14951n5lek97eDTodYfiF4UxeqL386krQ6eduscPIrin1114r
0w==
-----END CERTIFICATE-----`,
		URL:                       tinfo.ldapAddr,
		BaseDN:                    "DC=pensando,DC=io",
		BindDN:                    "CN=admin,DC=pensando,DC=io",
		BindPassword:              "pensando",
		UserAttribute:             "cn",
		UserObjectClassAttribute:  "organizationalPerson",
		GroupAttribute:            "ou",
		GroupObjectClassAttribute: "groupOfNames",
		TenantAttribute:           "l",
		ReferralServer:            "openldap",
		LdapServer:                "openldapref",
		LdapUser:                  "testuser",
		LdapUserGroupsDN:          []string{"cn=Administrators,dc=pensando,dc=io"},
		LdapUserPassword:          "pensando",
		ReferralUser:              "testReferral",
		ReferralUserDN:            "cn=testReferral,dc=pensando,dc=io",
		ReferralUserGroupDN:       "cn=AdministratorsReferral,dc=pensando,dc=io",
		ReferralUserPassword:      "pensando",
	}
}

func setupOpenLdap() {
	var err error
	config := getOpenLdapConfig()
	// start ldap server
	tinfo.ldapAddr, err = StartOpenLdapServer(config.LdapServer)
	if err != nil {
		log.Errorf("Error creating LDAP Server: %v", err)
		os.Exit(-1)
	}
	// start referral server
	tinfo.referralAddr, err = StartOpenLdapServer(config.ReferralServer)
	if err != nil {
		StopOpenLdapServer(config.LdapServer)
		log.Errorf("Error creating referral LDAP Server: %v", err)
		os.Exit(-1)
	}
	// create test ldap user
	err = CreateLdapUser(tinfo.ldapAddr, config.LdapUser, config.LdapUserPassword, testTenant, config.LdapUserGroupsDN)
	if err != nil {
		shutdownOpenLdap()
		log.Errorf("Error creating test ldap user: %v", err)
		os.Exit(-1)
	}
	// create testReferral ldap user in referral server
	err = CreateLdapUser(tinfo.referralAddr, config.ReferralUser, config.ReferralUserPassword, testTenant, []string{config.ReferralUserGroupDN})
	if err != nil {
		shutdownOpenLdap()
		log.Errorf("Error creating test ldap user: %v", err)
		os.Exit(-1)
	}
	// create testReferral ldap group in referral server
	err = CreateGroup(tinfo.referralAddr, config.ReferralUserGroupDN, config.LdapUserGroupsDN, []string{config.ReferralUserDN})
	if err != nil {
		shutdownOpenLdap()
		log.Errorf("Error creating testReferral ldap group in referral server: %v", err)
		os.Exit(-1)
	}
	// create referral entry in ldap server
	err = CreateReferral(tinfo.ldapAddr, config.ReferralUser, "ldap://"+tinfo.referralAddr+"/"+config.BaseDN)
	if err != nil {
		shutdownOpenLdap()
		log.Errorf("Error creating testReferral referral entry in ldap server: %v", err)
		os.Exit(-1)
	}
}

func shutdownOpenLdap() {
	config := getOpenLdapConfig()
	StopOpenLdapServer(config.LdapServer)
	StopOpenLdapServer(config.ReferralServer)
}

func TestAuthenticate(t *testing.T) {
	t.Skip()
	config := getOpenLdapConfig()
	testAuthenticate(t, config)
}

func TestIncorrectPasswordAuthentication(t *testing.T) {
	t.Skip()
	config := getOpenLdapConfig()
	testIncorrectPasswordAuthentication(t, config)
}

func TestIncorrectUserAuthentication(t *testing.T) {
	t.Skip()
	config := getOpenLdapConfig()
	testIncorrectUserAuthentication(t, config)
}

func TestMissingLdapAttributeMapping(t *testing.T) {
	t.Skip()
	config := getOpenLdapConfig()
	testMissingLdapAttributeMapping(t, config)
}

func TestIncorrectLdapAttributeMapping(t *testing.T) {
	t.Skip()
	config := getOpenLdapConfig()
	testIncorrectLdapAttributeMapping(t, config)
}

func TestIncorrectBaseDN(t *testing.T) {
	t.Skip()
	config := getOpenLdapConfig()
	testIncorrectBaseDN(t, config)
}

func TestIncorrectBindPassword(t *testing.T) {
	t.Skip()
	config := getOpenLdapConfig()
	testIncorrectBindPassword(t, config)
}

func TestDisabledLdapAuthenticator(t *testing.T) {
	t.Skip()
	config := getOpenLdapConfig()
	testDisabledLdapAuthenticator(t, config)
}

func TestReferral(t *testing.T) {
	t.Skip()
	config := getOpenLdapConfig()
	for testtype, ldapconf := range authenticationPoliciesData(config) {
		_, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{Enabled: true}, ldapconf)
		if err != nil {
			t.Errorf("err %s in CreateAuthenticationPolicy", err)
			return
		}
		// create password authenticator
		authenticator := ldap.NewLdapAuthenticator(ldapconf)

		// authenticate
		autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: config.ReferralUser, Password: config.ReferralUserPassword})
		DeleteAuthenticationPolicy(tinfo.apicl)

		Assert(t, ok, fmt.Sprintf("[%v] Unsuccessful ldap user authentication", testtype))
		Assert(t, autheduser.Name == config.ReferralUser, fmt.Sprintf("[%v] User returned by ldap authenticator didn't match user being authenticated", testtype))
		Assert(t, autheduser.Spec.GetType() == auth.UserSpec_EXTERNAL.String(), fmt.Sprintf("[%v] User created is not of type EXTERNAL", testtype))
		returnedGroups := autheduser.Status.GetUserGroups()
		sort.Strings(returnedGroups)
		var expectedGroups []string
		expectedGroups = append(expectedGroups, config.LdapUserGroupsDN...)
		expectedGroups = append(expectedGroups, config.ReferralUserGroupDN)
		sort.Strings(expectedGroups)
		Assert(t, reflect.DeepEqual(autheduser.Status.GetUserGroups(), expectedGroups),
			fmt.Sprintf("[%v] Incorrect user group returned, expected [%v], got [%v]", testtype, expectedGroups, returnedGroups))
		AssertOk(t, err, fmt.Sprintf("[%v] Error authenticating user", testtype))
	}
}
