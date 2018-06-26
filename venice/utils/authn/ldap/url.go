package ldap

import (
	"errors"
	"net/url"
	"strings"

	"gopkg.in/ldap.v2"
)

// ScopeType defines scope for searching ldap
type ScopeType string

// Scope choices
const (
	// BASE for a base object search
	BASE ScopeType = "base"
	// SUB for a subtree search
	SUB ScopeType = "sub"
	// ONE for a one-level search
	ONE ScopeType = "one"
)

func (s ScopeType) String() string {
	return string(s)
}

var (
	// ErrIncorrectProtocolScheme error is thrown when LDAP URL doesn't start with ldap:// scheme
	ErrIncorrectProtocolScheme = errors.New("invalid protocol scheme in ldap url")
)

// URL holds extracted information from ldap referral
type URL struct {
	Addr string
	// BaseDN must be present in a SearchResultReference (rfc4511)
	BaseDN string
	// Scope could be base, sub, one (rfc4516)
	Scope  ScopeType
	Filter string
}

// ParseLdapURL parses ldap referral to extract host:port, scope of search and filter.
func ParseLdapURL(referral, defaultDN string, defaultScope ScopeType, defaultFilter string) (*URL, error) {
	if !strings.HasPrefix(referral, "ldap://") {
		return nil, ErrIncorrectProtocolScheme
	}
	parsedURL, err := url.Parse(referral)
	if err != nil {
		return nil, err
	}
	ldapURL := &URL{}
	ldapURL.Addr = parsedURL.Host
	// Remove leading slash
	dn, err := ldap.ParseDN(strings.TrimLeft(parsedURL.Path, "/"))
	if err != nil {
		return nil, err
	}
	ldapURL.BaseDN = dnString(dn)
	// set dn from default if not set
	if ldapURL.BaseDN == "" && defaultDN != "" {
		if err := ldapURL.setDefaultDN(defaultDN); err != nil {
			return nil, err
		}
	}

	query, err := url.PathUnescape(parsedURL.RawQuery)
	if err != nil {
		return nil, err
	}

	// set default scope type
	if defaultScope == "" {
		defaultScope = BASE // rfc4516
	}
	ldapURL.Scope = defaultScope

	// set default filter
	if defaultFilter == "" {
		defaultFilter = "(objectClass=*)" // rfc4516
	}
	ldapURL.Filter = defaultFilter
	vals := strings.Split(query, "?")
	for i, val := range vals {
		switch i {
		case 1:
			if val != "" {
				ldapURL.Scope = ScopeType(val)
			}
		case 2:
			if val != "" {
				ldapURL.Filter = val
			}
		}
	}
	return ldapURL, nil
}

// GetScope returns integer mapping for scope type const as defined in LDAP library
func (l *URL) GetScope() int {
	switch l.Scope {
	case BASE:
		return ldap.ScopeBaseObject
	case SUB:
		return ldap.ScopeWholeSubtree
	case ONE:
		return ldap.ScopeSingleLevel
	default:
		return ldap.ScopeBaseObject
	}
}

// setDefaultDN  validates the supplied default base DN and assign it to member field.
func (l *URL) setDefaultDN(defaultDN string) error {
	dn, err := ldap.ParseDN(defaultDN)
	if err != nil {
		return err
	}
	l.BaseDN = dnString(dn)
	return nil
}

// String returns referral string
func (l *URL) String() string {
	return "ldap://" + l.Addr + "/" + l.BaseDN + "??" + l.Scope.String() + "?" + l.Filter
}

func dnString(dn *ldap.DN) string {
	var val string
	for _, rdn := range dn.RDNs {
		for _, attr := range rdn.Attributes {
			val = val + attr.Type + "=" + attr.Value + ","
		}
	}
	return strings.TrimRight(val, ",")
}
