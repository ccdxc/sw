// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package tokenauth

import (
	"bufio"
	"bytes"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/tls"
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"net/url"
	"strings"
	"time"

	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	audiencePath = "audience"
)

// MakeNodeToken creates a node auth token with the supplied parameters and signs is with the input CA
func MakeNodeToken(ca *certmgr.CertificateAuthority, clusterName string, audience []string, notBefore, notAfter time.Time) (string, error) {
	privateKey, err := ecdsa.GenerateKey(elliptic.P384(), rand.Reader)
	if err != nil {
		return "", fmt.Errorf("Error generating private key: %v", err)
	}

	var URIs []*url.URL
	if len(audience) > 0 {
		audienceURIStr := fmt.Sprintf("venice://%s/%s?%s", clusterName, audiencePath, strings.Join(audience, "&"))
		audienceURI, err := url.Parse(audienceURIStr)
		if err == nil {
			URIs = append(URIs, audienceURI)
		} else {
			log.Errorf("Error parsing audience URI %s: %v", audienceURIStr, err)
		}
	}

	template := &x509.CertificateRequest{URIs: URIs}
	csrBytes, err := x509.CreateCertificateRequest(rand.Reader, template, privateKey)
	if err != nil {
		return "", fmt.Errorf("Error creating CSR: %v", err)
	}
	csr, err := x509.ParseCertificateRequest(csrBytes)
	if err != nil {
		return "", fmt.Errorf("Error parsing CSR: %v", err)
	}

	cert, err := ca.Sign(csr, certs.WithNotBefore(notBefore), certs.WithNotAfter(notAfter),
		certs.WithKeyUsage(x509.KeyUsageDigitalSignature, []x509.ExtKeyUsage{x509.ExtKeyUsageClientAuth}))
	if err != nil {
		return "", fmt.Errorf("Error creating certificate: %v", err)
	}

	p8key, err := x509.MarshalPKCS8PrivateKey(privateKey)
	if err != nil {
		return "", fmt.Errorf("Error marshaling PKCS8 private key: %v", err)
	}

	var token bytes.Buffer
	w := bufio.NewWriter(&token)

	// put certs first and key at the end, so that library function tls.X509KeyPair function
	// can load both certs and key from the same file
	blocks := []*pem.Block{
		&pem.Block{
			Type:  certs.CertificatePemBlockType,
			Bytes: cert.Raw,
		},
	}

	tc := ca.TrustChain()
	for _, c := range tc[:len(tc)-1] {
		blocks = append(blocks,
			&pem.Block{
				Type:  certs.CertificatePemBlockType,
				Bytes: c.Raw,
			})
	}

	blocks = append(blocks,
		&pem.Block{
			Type:  certs.PrivateKeyPemBlockType,
			Bytes: p8key,
		})

	for i, b := range blocks {
		err = pem.Encode(w, b)
		if err != nil {
			return "", fmt.Errorf("Error encoding block %d out of %d: %v", i, len(blocks), err)
		}
	}
	w.Flush()
	return token.String(), nil
}

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
	var audience []string
	for _, attr := range cert.URIs {
		switch attr.Path {
		case "/" + audiencePath:
			audience = append(audience, strings.Split(attr.RawQuery, "&")...)
		default:
			log.Errorf("Found unknwon attribute in token: %s", attr)
		}
	}
	return audience, nil
}
