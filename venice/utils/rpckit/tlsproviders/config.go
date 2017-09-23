// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package tlsproviders

import (
	"crypto/tls"
	"crypto/x509"
)

// This file contains the common TLS configuration templates that all providers start from

const minTLSVersion = tls.VersionTLS12

func getTLSServerConfig(serverName string, cert *tls.Certificate, roots *x509.CertPool) *tls.Config {
	// tls.Config template for servers
	tlsConfig := tls.Config{
		MinVersion: minTLSVersion,
		ClientAuth: tls.RequireAndVerifyClientCert,
		ServerName: serverName,
		ClientCAs:  roots,
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
		ServerName: serverName,
		RootCAs:    roots,
	}

	// Cert can be nil if it will be pulled on demand using GetClientCertificate or we don't authenticate client
	if cert != nil {
		tlsConfig.Certificates = []tls.Certificate{*cert}
	}

	return &tlsConfig
}
