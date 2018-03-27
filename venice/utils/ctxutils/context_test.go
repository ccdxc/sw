// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package ctxutils

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"fmt"
	"net"
	"testing"

	"github.com/pensando/sw/venice/utils/certs"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// TestGetFunctions tests the Getter using a mock context
// Tests under venice/utils/rpckit exercise the code using real gRPC context
func TestGetFunctions(t *testing.T) {
	addr := &net.IPAddr{
		IP: net.ParseIP("1.2.3.4"),
	}
	privateKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "GenerateKey fail")
	cert, err := certs.SelfSign(1, "hello", privateKey)
	AssertOk(t, err, "Failed to generate certificate")

	ctx1 := MakeMockContext(addr, nil)
	Assert(t, GetPeerAddress(ctx1) == addr.String(), fmt.Sprintf("Address mismatch, want: %v, have: %v", addr.String(), GetPeerAddress(ctx1)))
	Assert(t, GetPeerCertificate(ctx1) == nil, "Context with nil certificate returned non-empty certificate")
	Assert(t, GetPeerID(ctx1) == "", "Context with nil certificate returned non-empty peer ID")

	ctx2 := MakeMockContext(nil, cert)
	Assert(t, GetPeerAddress(ctx2) == "", "Context with nil address returned non-empty peer address")
	Assert(t, GetPeerCertificate(ctx2) == cert, "Certificate mismatch, want: %+v, have: %+v", cert, GetPeerCertificate(ctx2))
	Assert(t, GetPeerID(ctx2) == "hello", fmt.Sprintf("PeerID  mismatch, want: hello, have: %v", GetPeerID(ctx2)))

	ctx3 := MakeMockContext(addr, cert)
	Assert(t, GetPeerAddress(ctx3) == addr.String(), fmt.Sprintf("Address mismatch, want: %v, have: %v", addr.String(), GetPeerAddress(ctx3)))
	Assert(t, GetPeerCertificate(ctx3) == cert, "Certificate mismatch, want: %+v, have: %+v", cert, GetPeerCertificate(ctx3))
	Assert(t, GetPeerID(ctx3) == "hello", fmt.Sprintf("PeerID  mismatch, want: hello, have: %v", GetPeerID(ctx3)))

	ctx4 := MakeMockContext(nil, nil)
	Assert(t, GetPeerAddress(ctx4) == "", "Context with nil address returned non-empty peer address")
	Assert(t, GetPeerCertificate(ctx4) == nil, "Context with nil certificate returned non-empty certificate")
	Assert(t, GetPeerID(ctx4) == "", "Context with nil certificate returned non-empty peer ID")
}
