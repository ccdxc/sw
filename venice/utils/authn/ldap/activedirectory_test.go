package ldap

import (
	"fmt"
	"testing"

	"gopkg.in/ldap.v2"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestGetADPrimaryGroup(t *testing.T) {
	var rawSid []byte
	rawSid = append(rawSid, 1, 1)
	idAuthority, err := authn.CreateSecret(10)
	AssertOk(t, err, "unable to generate random id authority bytes")
	rawSid = append(rawSid, idAuthority...)
	tests := []struct {
		name  Testcase
		entry *ldap.Entry
		group string
		err   error
	}{
		{
			name: ADPrimaryGroup,
			entry: &ldap.Entry{
				DN: testUserDN,
				Attributes: []*ldap.EntryAttribute{
					ldap.NewEntryAttribute(PrimaryGroupID, []string{adPrimaryGroupID}),
					{
						Name: ObjectSid,
						ByteValues: [][]byte{
							rawSid,
						},
					},
				},
			},
			group: adPrimaryGroupDN,
			err:   nil,
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
		authenticator := &authenticator{
			ldapConfig:      ldapConf,
			getConnectionFn: getMockConnectionGetter(test.name),
		}
		group, err := authenticator.getADPrimaryGroup("ldap://"+ldapConf.Domains[0].Servers[0].Url, ldapConf.Domains[0].Servers[0].TLSOptions, test.entry)
		Assert(t, test.err == err, fmt.Sprintf("[%v] test failed, err: %v", test.name, err))
		Assert(t, group == test.group, fmt.Sprintf("[%v] test failed,, expected group [%v], got [%v]", test.name, test.group, group))
	}
}
