package auth

import (
	"fmt"
	"reflect"
	"sort"
	"testing"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn/ldap"
	"github.com/pensando/sw/venice/utils/log"

	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	ldapUserGroupDN = "cn=Administrators,dc=pensando,dc=io"
)

func getOpenLdapConfig() *LdapConfig {
	return &LdapConfig{
		ServerName:                ServerName,
		TrustedCerts:              TrustedCerts,
		URL:                       tinfo.ldapAddr,
		BaseDN:                    BaseDN,
		BindDN:                    BindDN,
		BindPassword:              BindPassword,
		UserAttribute:             UserAttribute,
		UserObjectClassAttribute:  UserObjectClassAttribute,
		GroupAttribute:            GroupAttribute,
		GroupObjectClassAttribute: GroupObjectClassAttribute,
		TenantAttribute:           TenantAttribute,
		ReferralServer:            "openldap",
		LdapServer:                "openldapref",
		LdapUser:                  "testuser",
		LdapUserGroupsDN:          []string{ldapUserGroupDN},
		LdapUserPassword:          "pensando",
		ReferralUser:              "testReferral",
		ReferralUserDN:            fmt.Sprintf("cn=%s,%s", "testReferral", BaseDN),
		ReferralUserGroupDN:       fmt.Sprintf("cn=%s,%s", "AdministratorsReferral", BaseDN),
		ReferralUserPassword:      "pensando",
	}
}

func setupOpenLdap() error {
	var err error
	config := getOpenLdapConfig()
	var usedPort int
	// start ldap server
	if !CheckEventually(func() (bool, interface{}) {
		tinfo.ldapAddr, usedPort, err = StartOpenLdapServer(config.LdapServer)
		if err != nil {
			return false, err
		}
		return true, nil
	}, "15s", "45s") {
		log.Errorf("Error creating LDAP Server: %v", err)
		return err
	}
	// start referral server
	if !CheckEventually(func() (bool, interface{}) {
		tinfo.referralAddr, _, err = StartOpenLdapServer(config.ReferralServer, usedPort)
		if err != nil {
			return false, err
		}
		return true, nil
	}, "15s", "45s") {
		StopOpenLdapServer(config.LdapServer)
		log.Errorf("Error creating referral LDAP Server: %v", err)
		return err
	}
	// create test ldap user
	if !CheckEventually(func() (bool, interface{}) {
		if err = CreateLdapUser(tinfo.ldapAddr, config.LdapUser, config.LdapUserPassword, testTenant, config.LdapUserGroupsDN); err != nil {
			return false, err
		}
		return true, nil
	}, "100ms", "60s") {
		shutdownOpenLdap()
		log.Errorf("Error creating test ldap user %s: %v", config.LdapUser, err)
		return err
	}
	// create test Administrators group
	if !CheckEventually(func() (bool, interface{}) {
		if err = CreateGroup(tinfo.ldapAddr, ldapUserGroupDN, []string{}, []string{fmt.Sprintf("cn=%s,%s", config.LdapUser, config.BaseDN)}); err != nil {
			return false, err
		}
		return true, nil
	}, "100ms", "60s") {
		shutdownOpenLdap()
		log.Errorf("Error creating test user ldap group in ldap server: %v", err)
		return err
	}
	// create testReferral ldap user in referral server
	if !CheckEventually(func() (bool, interface{}) {
		if err = CreateLdapUser(tinfo.referralAddr, config.ReferralUser, config.ReferralUserPassword, testTenant, []string{config.ReferralUserGroupDN}); err != nil {
			return false, err
		}
		return true, nil
	}, "100ms", "60s") {
		shutdownOpenLdap()
		log.Errorf("Error creating test referral ldap user: %v", err)
		return err
	}
	// create test Administrators group in referral ldap
	if !CheckEventually(func() (bool, interface{}) {
		if err = CreateGroup(tinfo.referralAddr, ldapUserGroupDN, []string{}, []string{config.ReferralUserGroupDN}); err != nil {
			return false, err
		}
		return true, nil
	}, "100ms", "60s") {
		shutdownOpenLdap()
		log.Errorf("Error creating Administrators group in referral ldap server: %v", err)
		return err
	}
	// create testReferral ldap group in referral server
	if !CheckEventually(func() (bool, interface{}) {
		if err = CreateGroup(tinfo.referralAddr, config.ReferralUserGroupDN, config.LdapUserGroupsDN, []string{config.ReferralUserDN}); err != nil {
			return false, err
		}
		return true, nil
	}, "100ms", "60s") {
		shutdownOpenLdap()
		log.Errorf("Error creating testReferral ldap group in referral server: %v", err)
		return err
	}
	// create referral entry in ldap server
	if !CheckEventually(func() (bool, interface{}) {
		if err = CreateReferral(tinfo.ldapAddr, config.ReferralUser, "ldap://"+tinfo.referralAddr+"/"+config.BaseDN); err != nil {
			return false, err
		}
		return true, nil
	}, "100ms", "60s") {
		shutdownOpenLdap()
		log.Errorf("Error creating testReferral referral entry in ldap server: %v", err)
		return err
	}
	return nil
}

func shutdownOpenLdap() {
	config := getOpenLdapConfig()
	StopOpenLdapServer(config.LdapServer)
	StopOpenLdapServer(config.ReferralServer)
}

func TestAuthenticate(t *testing.T) {
	config := getOpenLdapConfig()
	testAuthenticate(t, config)
}

func TestIncorrectPasswordAuthentication(t *testing.T) {
	config := getOpenLdapConfig()
	testIncorrectPasswordAuthentication(t, config)
}

func TestIncorrectUserAuthentication(t *testing.T) {
	config := getOpenLdapConfig()
	testIncorrectUserAuthentication(t, config)
}

func TestIncorrectLdapAttributeMapping(t *testing.T) {
	config := getOpenLdapConfig()
	testIncorrectLdapAttributeMapping(t, config)
}

func TestIncorrectBaseDN(t *testing.T) {
	config := getOpenLdapConfig()
	testIncorrectBaseDN(t, config)
}

func TestIncorrectBindPassword(t *testing.T) {
	config := getOpenLdapConfig()
	testIncorrectBindPassword(t, config)
}

func TestDisabledLdapAuthenticator(t *testing.T) {
	config := getOpenLdapConfig()
	testDisabledLdapAuthenticator(t, config)
}

func TestReferral(t *testing.T) {
	config := getOpenLdapConfig()
	for testtype, ldapconf := range authenticationPoliciesData(config) {
		_, err := CreateAuthenticationPolicy(tinfo.apicl, &auth.Local{}, ldapconf)
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
		Assert(t, autheduser.Spec.GetType() == auth.UserSpec_External.String(), fmt.Sprintf("[%v] User created is not of type EXTERNAL", testtype))
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
