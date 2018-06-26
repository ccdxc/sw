package ldap

import (
	"fmt"
	"reflect"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestParseLdapUrl(t *testing.T) {
	tests := []struct {
		name          string
		url           string
		defaultDN     string
		defaultScope  ScopeType
		defaultFilter string
		expected      *URL
		err           bool
	}{
		{
			name: "baseDN with spaces, scope as subtree, filter with space",
			url:  "ldap://ldap1.example.net:6666/o=University%20of%20Michigan,c=US??sub?(cn=Babs%20Jensen)",
			expected: &URL{
				Addr:   "ldap1.example.net:6666",
				BaseDN: "o=University of Michigan,c=US",
				Scope:  SUB,
				Filter: "(cn=Babs Jensen)",
			},
			err: false,
		},
		{
			name: "basedDN with reserved character ?",
			url:  "ldap://ldap2.example.com/o=Question%3f,c=US?mail",
			expected: &URL{
				Addr:   "ldap2.example.com",
				BaseDN: "o=Question?,c=US",
				Scope:  BASE,
				Filter: "(objectClass=*)",
			},
			err: false,
		},
		{
			name: "basedDN with comma encoded",
			url:  "ldap://ldap.example.com/o=An%20Example%5C2C%20Inc.,c=US",
			expected: &URL{
				Addr:   "ldap.example.com",
				BaseDN: "o=An Example, Inc.,c=US",
				Scope:  BASE,
				Filter: "(objectClass=*)",
			},
			err: false,
		},
		{
			name: "filter with encoded octets",
			url:  "ldap://ldap3.example.com/o=Babsco,c=US???(four-octet=%5c00%5c00%5c00%5c04)",
			expected: &URL{
				Addr:   "ldap3.example.com",
				BaseDN: "o=Babsco,c=US",
				Scope:  BASE,
				Filter: "(four-octet=\\00\\00\\00\\04)",
			},
			err: false,
		},
		{
			name: "no host specified",
			url:  "ldap:///o=University%20of%20Michigan,c=US",
			expected: &URL{
				Addr:   "",
				BaseDN: "o=University of Michigan,c=US",
				Scope:  BASE,
				Filter: "(objectClass=*)",
			},
			err: false,
		},
		{
			name: "no baseDN specified",
			url:  "ldap://ldap3.example.com/",
			expected: &URL{
				Addr:   "ldap3.example.com",
				BaseDN: "",
				Scope:  BASE,
				Filter: "(objectClass=*)",
			},
			err: false,
		},
		{
			name:     "incorrect protocol scheme",
			url:      "http://ldap3.example.com/",
			expected: nil,
			err:      true,
		},
		{
			name:     "incorrect DN",
			url:      "ldap://ldap3.example.com/DC=example,=net",
			expected: nil,
			err:      true,
		},
		{
			name:     "invalid host",
			url:      "ldap://ldap3 example.com/",
			expected: nil,
			err:      true,
		},
		{
			name:     "invalid hex digits in filter",
			url:      "ldap://ldap3.example.com/o=Babsco,c=US???(cn=%zzzzz)",
			expected: nil,
			err:      true,
		},
		{
			name:          "default scope and filter",
			url:           "ldap://ldap1.example.net:6666/o=University%20of%20Michigan,c=US",
			defaultScope:  SUB,
			defaultFilter: "(cn=Babs Jensen)",
			expected: &URL{
				Addr:   "ldap1.example.net:6666",
				BaseDN: "o=University of Michigan,c=US",
				Scope:  SUB,
				Filter: "(cn=Babs Jensen)",
			},
			err: false,
		},
		{
			name:          "default scope and filter with query string",
			url:           "ldap://ldap1.example.net:6666/o=University%20of%20Michigan,c=US???",
			defaultScope:  SUB,
			defaultFilter: "(cn=Babs Jensen)",
			expected: &URL{
				Addr:   "ldap1.example.net:6666",
				BaseDN: "o=University of Michigan,c=US",
				Scope:  SUB,
				Filter: "(cn=Babs Jensen)",
			},
			err: false,
		},
		{
			name:          "default DN",
			url:           "ldap://ldap1.example.net:6666/",
			defaultDN:     "o=University of Michigan,c=US",
			defaultScope:  SUB,
			defaultFilter: "(cn=Babs Jensen)",
			expected: &URL{
				Addr:   "ldap1.example.net:6666",
				BaseDN: "o=University of Michigan,c=US",
				Scope:  SUB,
				Filter: "(cn=Babs Jensen)",
			},
			err: false,
		},
	}
	for _, test := range tests {
		ldapURL, err := ParseLdapURL(test.url, test.defaultDN, test.defaultScope, test.defaultFilter)
		Assert(t, test.err == (err != nil), fmt.Sprintf("[%v] test failed for ldap url [%s]", test.name, test.url))
		Assert(t, reflect.DeepEqual(test.expected, ldapURL), fmt.Sprintf("[%v] test failed, expected [%v], got [%v]", test.name, test.expected, ldapURL))
	}
}
