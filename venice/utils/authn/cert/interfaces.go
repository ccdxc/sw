package cert

import (
	"crypto/tls"
)

// KeyPair is an interface to create TLS configuration. Implementations can use different mechanisms to store and access certificate and private key.
type KeyPair interface {
	// TLSConfig creates TLS Configuration
	TLSConfig() *tls.Config
	// Start re-initializes KeyPair after it Stop() has been called
	Start()
	// Stop un-initializes KeyPair
	Stop()
}
