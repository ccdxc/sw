package ldap

import (
	"errors"
	"fmt"
	"testing"

	"github.com/pensando/sw/api/generated/auth"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestValidateLdapConfig(t *testing.T) {
	tests := []struct {
		name string
		in   *auth.Ldap
		errs []error
	}{
		{
			name: "Invalid LDAP config",
			in: &auth.Ldap{
				Domains: []*auth.LdapDomain{
					{
						AttributeMapping: &auth.LdapAttributeMapping{},
					},
				},
			},
			errs: []error{errors.New("base DN not defined"),
				errors.New("bind DN not defined"),
				errors.New("user attribute mapping not defined"),
				errors.New("user object class not defined"),
				errors.New("group attribute mapping not defined"),
				errors.New("group object class not defined"),
				errors.New("ldap server not defined")},
		},
		{
			name: "No LDAP attribute mapping",
			in: &auth.Ldap{
				Domains: []*auth.LdapDomain{{}},
			},
			errs: []error{errors.New("base DN not defined"),
				errors.New("bind DN not defined"),
				errors.New("ldap attributes mapping not defined"),
				errors.New("ldap server not defined")},
		},
		{
			name: "Valid LDAP Config",
			in: &auth.Ldap{
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

			errs: []error{},
		},
		{
			name: "Valid LDAP Config, takes default port",
			in: &auth.Ldap{
				Domains: []*auth.LdapDomain{
					{
						Servers: []*auth.LdapServer{
							{
								Url: "localhost",
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

			errs: []error{},
		},
		{
			name: "Missing LDAP Server",
			in: &auth.Ldap{
				Domains: []*auth.LdapDomain{
					{
						Servers: []*auth.LdapServer{},

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
			errs: []error{errors.New("ldap server not defined")},
		},
		{
			name: "Valid LDAP config, missing server URL",
			in: &auth.Ldap{
				Domains: []*auth.LdapDomain{
					{
						Servers: []*auth.LdapServer{
							{
								Url: "",
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

			errs: []error{},
		},
		{
			name: "Invalid TLS Config",
			in: &auth.Ldap{
				Domains: []*auth.LdapDomain{
					{
						Servers: []*auth.LdapServer{
							{
								Url: "localhost:389",
								TLSOptions: &auth.TLSOptions{
									StartTLS:                   true,
									SkipServerCertVerification: false,
									ServerName:                 "",
									TrustedCerts:               "",
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

			errs: []error{ErrSSLConfig, errors.New("remote server name not defined")},
		},
		{
			name: "Missing TLS Options ",
			in: &auth.Ldap{
				Domains: []*auth.LdapDomain{
					{
						Servers: []*auth.LdapServer{
							{
								Url:        "localhost:389",
								TLSOptions: nil,
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

			errs: []error{},
		},
		{
			name: "No ldap domains",
			in: &auth.Ldap{
				Domains: []*auth.LdapDomain{},
			},

			errs: []error{errors.New("ldap domain not defined")},
		},
		{
			name: "More than one ldap domains",
			in: &auth.Ldap{
				Domains: []*auth.LdapDomain{{}, {}},
			},

			errs: []error{errors.New("only one ldap domain is supported")},
		},
		{
			name: "no ldap authenticator",
			in:   nil,
			errs: []error{errors.New("ldap authenticator config not defined")},
		},
	}

	for _, test := range tests {
		errs := ValidateLdapConfig(test.in)
		SortErrors(errs)
		SortErrors(test.errs)
		Assert(t, len(errs) == len(test.errs), fmt.Sprintf("[%s] test failed, expected errors [%#v], got [%#v]", test.name, test.errs, errs))
		for i, err := range errs {
			Assert(t, err.Error() == test.errs[i].Error(), fmt.Sprintf("[%s] test failed, expected errors [%#v], got [%#v]", test.name, test.errs[i], errs[i]))
		}
	}
}
