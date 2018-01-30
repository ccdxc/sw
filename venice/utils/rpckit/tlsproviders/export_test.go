// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package tlsproviders

import (
	"crypto/tls"
)

// Export private functions/methods for testing
// This is needed because tests have been put in a dedicated namespace to avoid circular dependencies

func (p *CMDBasedProvider) GetServerCertificate(clientHelloInfo *tls.ClientHelloInfo) (*tls.Certificate, error) {
	return p.getServerCertificate(clientHelloInfo)
}
