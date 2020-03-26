package readutils

import (
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"strings"

	"github.com/pensando/sw/venice/utils/log"
)

const veniceURIScheme = "venice"

const (
	// AudiencePath is the relative path used within SAN/URI field of a certificate
	AudiencePath = "audience"
)

// ParseNodeToken converts a node auth token string into a tls.Certificate struc
func ParseNodeToken(token string) (tls.Certificate, error) {
	pemBlocks := []byte(token)
	return tls.X509KeyPair(pemBlocks, pemBlocks)
}

// GetNodeTokenAttributes returns audience in a token
func GetNodeTokenAttributes(token string) ([]string, error) {
	tlsCert, err := ParseNodeToken(token)
	if err != nil {
		return nil, fmt.Errorf("Error parsing token: %v", err)
	}
	if len(tlsCert.Certificate) < 1 {
		return nil, fmt.Errorf("No certificates found in token")
	}
	cert, err := x509.ParseCertificate(tlsCert.Certificate[0])
	if err != nil {
		return nil, fmt.Errorf("Error parsing certificate: %v", err)
	}
	return ExtractAudienceFromVeniceCert(cert)
}

// ExtractAudienceFromVeniceCert extracts audience from a venice issued certificate provided as argument
func ExtractAudienceFromVeniceCert(cert *x509.Certificate) ([]string, error) {
	var audience []string
	for _, attr := range cert.URIs {
		switch attr.Scheme {
		case veniceURIScheme:
			switch attr.Path {
			case "/" + AudiencePath:
				audience = append(audience, strings.Split(attr.RawQuery, "&")...)
			default:
				log.Errorf("Found unknown path '%s' in URI: %v", attr.Path, attr)
			}
		default:
			log.Errorf("Found unknown scheme '%s' in URI: %v", attr.Scheme, attr)
		}
	}
	return audience, nil
}
