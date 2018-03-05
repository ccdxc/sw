// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package tlsproviders

import (
	"crypto/tls"
	"crypto/x509"
	"net"
)

// This file contains the common TLS configuration templates that all providers start from

const (
	minTLSVersion = tls.VersionTLS12
	tlsCurve      = tls.CurveP256
)

var tlsCipherSuites = []uint16{
	tls.TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
	// Some tests are still using certificates with RSA keys, so we need an RSA cipher.
	// Nothing wrong with it, but plan is to keep only ECDSA, as it is faster.
	tls.TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
}

func getTLSServerConfig(serverName string, cert *tls.Certificate, roots *x509.CertPool) *tls.Config {
	// tls.Config template for servers
	tlsConfig := tls.Config{
		MinVersion:               minTLSVersion,
		ClientAuth:               tls.RequireAndVerifyClientCert,
		ClientCAs:                roots,
		PreferServerCipherSuites: true,
		SessionTicketsDisabled:   true,
		CipherSuites:             tlsCipherSuites,
		CurvePreferences:         []tls.CurveID{tlsCurve},
	}

	// Cert can be nil if it will be pulled on deman using GetCertificate
	if cert != nil {
		tlsConfig.Certificates = []tls.Certificate{*cert}
	}

	return &tlsConfig
}

func getTLSClientConfig(serverName string, cert *tls.Certificate, roots *x509.CertPool) *tls.Config {
	// tls.Config template for clients
	tlsConfig := tls.Config{
		MinVersion: minTLSVersion,
		RootCAs:    roots,
	}

	host, _, err := net.SplitHostPort(serverName)
	if err != nil {
		// not a URL, client has provided service name
		tlsConfig.ServerName = serverName
	} else {
		// generic URL
		tlsConfig.ServerName = host
	}

	// Cert can be nil if it will be pulled on demand using GetClientCertificate or we don't authenticate client
	if cert != nil {
		tlsConfig.Certificates = []tls.Certificate{*cert}
	}

	return &tlsConfig
}
