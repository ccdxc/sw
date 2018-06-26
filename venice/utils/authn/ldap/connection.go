package ldap

import (
	"crypto/tls"
	"crypto/x509"

	"gopkg.in/ldap.v2"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/log"
)

// connectionGetter gets a connection to LDAP
type connectionGetter func(addr string, tlsOptions *auth.TLSOptions) (connection, error)

// connection abstracts out ldap connection
type connection interface {
	// Bind authenticates user
	Bind(username, password string) error
	// Search searches ldap with the given search request
	Search(*ldap.SearchRequest) (*ldap.SearchResult, error)
	// Close closes LDAP connection
	Close()
}

// getConnection is of type connectionGetter that uses LDAP library to get connection
func getConnection(addr string, tlsOptions *auth.TLSOptions) (connection, error) {
	conn, err := ldap.Dial("tcp", addr)
	if err != nil {
		log.Errorf("Unable to establish ldap connection with [%q], Err: %v", addr, err)
		return nil, err
	}
	if tlsOptions.GetStartTLS() {
		trustedCerts := tlsOptions.GetTrustedCerts()
		var certpool *x509.CertPool
		if trustedCerts != "" {
			certpool = x509.NewCertPool()
			if !certpool.AppendCertsFromPEM([]byte(trustedCerts)) {
				log.Errorf("Unable to parse trusted certificates from LDAP config and create trusted cert pool: %v", trustedCerts)
				conn.Close()
				return nil, ErrSSLConfig
			}
		}
		if err := conn.StartTLS(
			&tls.Config{
				InsecureSkipVerify: tlsOptions.GetSkipServerCertVerification(),
				ServerName:         tlsOptions.GetServerName(),
				RootCAs:            certpool, // if certpool is nil TLS uses the host's root CA set.
			}); err != nil {
			log.Errorf("Unable to establish TLS connection to LDAP [%s], Err: %v", addr, err)
			conn.Close()
			return nil, err
		}
	}
	return conn, nil
}
