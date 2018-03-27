// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package ctxutils

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"net"

	"google.golang.org/grpc/credentials"
	"google.golang.org/grpc/peer"
)

/* 	Utilities to extract useful content from Go context

This is an example of what we start from:

context.Background.WithCancel.WithValue(
	peer.peerKey{},
	peer.Peer{
		Addr:(*net.TCPAddr)(0xc424380c60),
		AuthInfo:credentials.TLSInfo{
			State:tls.ConnectionState{
				Version:0x303,
				HandshakeComplete:true,
				DidResume:false,
				CipherSuite:0xc02b,
				NegotiatedProtocol:\"h2\",
				NegotiatedProtocolIsMutual:true,
				ServerName:\"SecretsV1GwService\",
				PeerCertificates:[]*x509.Certificate{
					(*x509.Certificate)(0xc427815600),
					(*x509.Certificate)(0xc427815b80)},
				VerifiedChains:[][]*x509.Certificate{
						[]*x509.Certificate{
							(*x509.Certificate)(0xc427815600),
							(*x509.Certificate)(0xc4200fcc00)}
				},
				SignedCertificateTimestamps:[][]uint8(nil),
				OCSPResponse:[]uint8(nil),
				TLSUnique:[]uint8{0x41, 0x6a, 0xa8, 0xbe, 0x26, 0x74, 0xb7, 0xb0, 0x3b, 0xe1, 0xd7, 0x97}
			}
		}
	})

	// other values, not shown here, make available the URL and other HTTP headers
*/

// GetPeerAddress returns a stringified version of peer IP address
func GetPeerAddress(ctx context.Context) string {
	peer, ok := peer.FromContext(ctx)
	if !ok || peer.Addr == net.Addr(nil) {
		return ""
	}
	return peer.Addr.String()
}

// GetPeerCertificate returns the certificate presented by the client
// only returns the first certificate of the first validated chain
func GetPeerCertificate(ctx context.Context) *x509.Certificate {
	peer, ok := peer.FromContext(ctx)
	if !ok || peer.AuthInfo == nil {
		return nil
	}
	tlsInfo, ok := peer.AuthInfo.(credentials.TLSInfo)
	if !ok {
		return nil
	}
	if len(tlsInfo.State.VerifiedChains) < 1 || len(tlsInfo.State.VerifiedChains[0]) < 1 {
		return nil
	}
	return tlsInfo.State.VerifiedChains[0][0]
}

// GetPeerID returns the first SAN from validate peer certificate or empty string if not available
func GetPeerID(ctx context.Context) string {
	cert := GetPeerCertificate(ctx)
	if cert != nil && len(cert.DNSNames) > 0 {
		return cert.DNSNames[0]
	}
	return ""
}

// MakeMockContext returns a valid peer context containing the supplied address and certificate
// ONLY TO BE USED FOR TESTING
func MakeMockContext(addr net.Addr, cert *x509.Certificate) context.Context {
	var tlsInfo credentials.TLSInfo

	if cert != nil {
		verifiedChains := make([][]*x509.Certificate, 1)
		verifiedChains[0] = make([]*x509.Certificate, 1)
		verifiedChains[0][0] = cert

		tlsInfo = credentials.TLSInfo{
			State: tls.ConnectionState{
				VerifiedChains:   verifiedChains,
				PeerCertificates: verifiedChains[0],
			},
		}
	}

	peerCtx := &peer.Peer{
		Addr:     addr,
		AuthInfo: tlsInfo,
	}

	return peer.NewContext(context.Background(), peerCtx)
}
