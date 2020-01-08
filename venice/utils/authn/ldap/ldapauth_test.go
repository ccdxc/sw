package ldap

import (
	"fmt"
	"reflect"
	"sort"
	"testing"

	"gopkg.in/ldap.v2"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// getDefaultAuthenticationPolicy gets an authentication policy with LDAP with TLS enabled
func getDefaultAuthenticationPolicy() *auth.AuthenticationPolicy {
	// authn policy object
	return &auth.AuthenticationPolicy{
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
									Url: ldapURL,
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
		},
	}
}

func TestBind(t *testing.T) {
	tests := []struct {
		name     Testcase
		url      string
		username string
		password string
		entry    *ldap.Entry
		groups   []string
		err      error
	}{
		{
			name:     IncorrectReferralFormat,
			url:      "incorrect referral",
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleUserEntries,
		},
		{
			name:     IncorrectReferralFormat,
			url:      "", // to test no host name in url
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleUserEntries,
		},
		{
			name:     IncorrectBindDN,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleUserEntries,
		},
		{
			name:     IncorrectUserPassword,
			url:      ldapURL,
			username: testUser,
			password: "wrongPassword",
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleUserEntries,
		},
		{
			name:     EmptyUserPassword,
			url:      ldapURL,
			username: testUser,
			password: "",
			entry:    nil,
			groups:   nil,
			err:      ErrEmptyUserPassword,
		},
		{
			name:     NonExistentUser,
			url:      ldapURL,
			username: "non existent",
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleUserEntries,
		},
		{
			name:     NoGroupMembership,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry: &ldap.Entry{
				DN: BaseDN,
			},
			groups: nil,
			err:    nil,
		},
		{
			name:     ConnectionError,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoServerAvailable,
		},
		{
			name:     FailedSearch,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleUserEntries,
		},
		{
			name:     GroupReferral,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry: &ldap.Entry{
				DN: testUserDN,
				Attributes: []*ldap.EntryAttribute{
					ldap.NewEntryAttribute(GroupAttribute, []string{networkAdminGroupDN}),
				},
			},
			groups: []string{networkAdminGroupDN},
			err:    nil,
		},
		{
			name:     LoopReferral,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleGroupEntries,
		},
		{
			name:     GroupHierarchy,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry: &ldap.Entry{
				DN: testUserDN,
				Attributes: []*ldap.EntryAttribute{
					ldap.NewEntryAttribute(GroupAttribute, []string{networkAdminGroupDN}),
				},
			},
			groups: []string{networkAdminGroupDN, securityAdminGroupDN},
			err:    nil,
		},
		{
			name:     UserReferral,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry: &ldap.Entry{
				DN: testUserDN,
				Attributes: []*ldap.EntryAttribute{
					ldap.NewEntryAttribute(GroupAttribute, []string{networkAdminGroupDN}),
				},
			},
			groups: []string{networkAdminGroupDN},
			err:    nil,
		},
		{
			name:     BindTimeout,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleUserEntries,
		},
	}

	ldapConf := &auth.Ldap{
		Domains: []*auth.LdapDomain{
			{
				Servers: []*auth.LdapServer{
					{
						Url: ldapURL,
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
	}
	for _, test := range tests {
		ldapConf.Domains[0].Servers[0].Url = test.url
		authenticator := &authenticator{
			ldapConfig:      ldapConf,
			getConnectionFn: getMockConnectionGetter(test.name),
		}
		entry, groups, err := authenticator.bind(test.username, test.password)
		Assert(t, test.err == err, fmt.Sprintf("[%v] test failed, err: %v", test.name, err))
		Assert(t, reflect.DeepEqual(test.entry, entry), fmt.Sprintf("[%v] test failed, expected entry [%v], got [%v]", test.name, test.entry, entry))
		// sort groups
		sort.Strings(test.groups)
		sort.Strings(groups)
		Assert(t, reflect.DeepEqual(test.groups, groups), fmt.Sprintf("[%v] test failed,, expected groups [%v], got [%v]", test.name, test.groups, groups))
	}
}

func TestAuthenticate(t *testing.T) {
	policy := getDefaultAuthenticationPolicy()
	authenticator := &authenticator{
		ldapConfig:      policy.Spec.Authenticators.Ldap,
		getConnectionFn: getMockConnectionGetter(SuccessfulAuth),
	}
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: testUser, Password: testPassword})
	Assert(t, ok, "Unsuccessful ldap user authentication")
	Assert(t, autheduser.Name == testUser, "User returned by ldap authenticator didn't match user being authenticated")
	Assert(t, autheduser.Spec.GetType() == auth.UserSpec_External.String(), "User created is not of type EXTERNAL")
	Assert(t, autheduser.Status.GetUserGroups()[0] == networkAdminGroupDN,
		fmt.Sprintf("Incorrect user group returned, expected [%s], got [%s]", networkAdminGroupDN, autheduser.Status.GetUserGroups()[0]))
	AssertOk(t, err, "Error authenticating user")
}
