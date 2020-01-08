package ldap

import (
	"crypto/x509"
	"errors"
	"net"
	"strconv"
	"strings"

	"github.com/pensando/sw/api/generated/auth"
)

const defaultLDAPPort int = 389

// AddDefaultPort checks if a port is present in URL of LDAP servers, else it adds default port
func AddDefaultPort(url string) (string, error) {
	_, _, err := net.SplitHostPort(url)
	if err != nil && strings.Contains(err.Error(), "missing port in address") {
		url = url + ":" + strconv.Itoa(defaultLDAPPort)
		return url, nil
	}
	return url, err
}

// ValidateLdapConfig validates ldap configuration
func ValidateLdapConfig(config *auth.Ldap) []error {
	var errs []error
	if config == nil {
		errs = append(errs, errors.New("ldap authenticator config not defined"))
		return errs
	}
	if len(config.Domains) > 1 {
		errs = append(errs, errors.New("only one ldap domain is supported"))
		return errs
	}
	if len(config.Domains) == 0 {
		errs = append(errs, errors.New("ldap domain not defined"))
		return errs
	}
	domain := config.Domains[0]
	if domain.BaseDN == "" {
		errs = append(errs, errors.New("base DN not defined"))
	}
	if domain.BindDN == "" {
		errs = append(errs, errors.New("bind DN not defined"))
	}
	if len(domain.Servers) == 0 {
		errs = append(errs, errors.New("ldap server not defined"))
	}
	for _, srv := range domain.Servers {
		if url, portErr := AddDefaultPort(srv.Url); portErr == nil {
			srv.Url = url
			urlNew := "ldap://" + srv.Url + "/" + domain.BaseDN
			_, err := ParseLdapURL(urlNew, "", SUB, "")
			if err != nil {
				errs = append(errs, err)
			}
		} else {
			errs = append(errs, portErr)
		}
		if srv.TLSOptions != nil && srv.TLSOptions.StartTLS && !srv.TLSOptions.SkipServerCertVerification {
			if srv.TLSOptions.ServerName == "" {
				errs = append(errs, errors.New("remote server name not defined"))
			}
			certpool := x509.NewCertPool()
			if !certpool.AppendCertsFromPEM([]byte(srv.TLSOptions.TrustedCerts)) {
				errs = append(errs, ErrSSLConfig)
			}
		}
	}
	if domain.AttributeMapping == nil {
		errs = append(errs, errors.New("ldap attributes mapping not defined"))
		return errs
	}
	if domain.AttributeMapping.User == "" {
		errs = append(errs, errors.New("user attribute mapping not defined"))
	}
	if domain.AttributeMapping.UserObjectClass == "" {
		errs = append(errs, errors.New("user object class not defined"))
	}
	if domain.AttributeMapping.Group == "" {
		errs = append(errs, errors.New("group attribute mapping not defined"))
	}
	if domain.AttributeMapping.GroupObjectClass == "" {
		errs = append(errs, errors.New("group object class not defined"))
	}

	return errs
}
