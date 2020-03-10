package ldap

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"time"

	"gopkg.in/ldap.v2"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils"
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
	addr, err := AddDefaultPort(addr) // ldap referral URL might not have port specified
	if err != nil {
		log.Errorf("Unable to add default port to ldap address: %v, err: %v", addr, err)
		return nil, err
	}

	conn, err := ldap.Dial("tcp", addr)
	if err != nil {
		log.Errorf("Unable to establish ldap connection with [%q], Err: %v", addr, err)
		return nil, err
	}
	if tlsOptions != nil && tlsOptions.GetStartTLS() {
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
	return &connectionWithTimeout{conn: conn}, nil
}

type connectionWithTimeout struct {
	conn connection
}

func (c *connectionWithTimeout) Bind(username, password string) error {
	bindFn := func(ctx context.Context) (interface{}, error) {
		if err := c.conn.Bind(username, password); err != nil {
			return false, err
		}
		return true, nil
	}
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	_, err := utils.ExecuteWithContext(ctx, bindFn)
	return err
}

// Search searches ldap with the given search request
func (c *connectionWithTimeout) Search(req *ldap.SearchRequest) (*ldap.SearchResult, error) {
	return c.conn.Search(req)
}

// Close closes LDAP connection
func (c *connectionWithTimeout) Close() {
	c.conn.Close()
}

// ConnectionChecker abstracts out LDAP connection check
type ConnectionChecker interface {
	// Connect checks if connection to LDAP is successful
	Connect(addr string, tlsOptions *auth.TLSOptions) (bool, error)
	// Bind checks if bind credentials are valid
	Bind(addr string, tlsOptions *auth.TLSOptions, bindDN, bindPassword string) (bool, error)
}

type connectionChecker struct {
	connGetter connectionGetter
}

func (c *connectionChecker) Connect(addr string, tlsOptions *auth.TLSOptions) (bool, error) {
	conn, err := c.connGetter(addr, tlsOptions)
	if err != nil {
		return false, err
	}
	conn.Close()
	return true, nil
}

func (c *connectionChecker) Bind(addr string, tlsOptions *auth.TLSOptions, bindDN, bindPassword string) (bool, error) {
	conn, err := c.connGetter(addr, tlsOptions)
	if err != nil {
		return false, err
	}
	defer conn.Close()
	if err := conn.Bind(bindDN, bindPassword); err != nil {
		return false, err
	}
	return true, nil
}

// NewConnectionChecker returns an implementation of ConnnectionChecker to test ldap connections
func NewConnectionChecker() ConnectionChecker {
	return &connectionChecker{
		connGetter: getConnection,
	}
}

func init() {
	// Sets timeout for ldap.Dial and ldap.DialTLS
	ldap.DefaultTimeout = 5 * time.Second
}
