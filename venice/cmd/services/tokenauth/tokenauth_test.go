package tokenauth

import (
	"crypto/x509"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/keymgr"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tokenauth/readutils"
)

func TestTokenAuth_GenerateNodeToken(t *testing.T) {
	be, err := keymgr.NewDefaultBackend("certmgr")
	AssertOk(t, err, "Error instantiating KeyMgr backend")
	defer be.Close()

	km, err := keymgr.NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")

	ca, err := certmgr.NewCertificateAuthority(km, true)
	AssertOk(t, err, "Error instantiating certificate authority")

	tokenAuthService := NewTokenAuthService("testCluster", ca)

	notBefore := time.Now()
	notBeforeTS := &api.Timestamp{
		Timestamp: types.Timestamp{
			Seconds: notBefore.Unix(),
		},
	}

	notAfter := time.Now().Add(time.Hour)
	notAfterTS := &api.Timestamp{
		Timestamp: types.Timestamp{
			Seconds: notAfter.Unix(),
		},
	}

	// when notBefore and notAfter timestamps are provided, verify cert contains the provided timestamps
	token, err := tokenAuthService.GenerateNodeToken([]string{"*"}, notBeforeTS, notAfterTS)
	AssertOk(t, err, "Unexpected GenerateNodeToken failure")
	tlsCert, err := readutils.ParseNodeToken(token)
	AssertOk(t, err, "Unexpected token parse failure")
	cert, err := x509.ParseCertificate(tlsCert.Certificate[0])
	AssertOk(t, err, "Unexppected ParseCertificate failure")
	AssertEquals(t, notBefore.Unix(), cert.NotBefore.Unix(), "NotBefore time in cert expected to be equal to GenerateNodeToken argument")
	AssertEquals(t, notAfter.Unix(), cert.NotAfter.Unix(), "NotAfter time in cert expected to be equal to GenerateNodeToken argument")

	// when notBefore and notAfter timestamps are not provided, verify cert contain default values
	token, err = tokenAuthService.GenerateNodeToken([]string{"*"}, nil, nil)
	AssertOk(t, err, "Unexpected GenerateNodeToken failure")
	tlsCert, err = readutils.ParseNodeToken(token)
	AssertOk(t, err, "Unexpected token parse failure")
	cert, err = x509.ParseCertificate(tlsCert.Certificate[0])
	AssertOk(t, err, "Unexppected ParseCertificate failure")
	AssertEquals(t, certs.BeginningOfTime.Unix(), cert.NotBefore.Unix(), "NotBefore time in cert expected to be equal to GenerateNodeToken argument")
	AssertEquals(t, certs.EndOfTime.Unix(), cert.NotAfter.Unix(), "NotAfter time in cert expected to be equal to GenerateNodeToken argument")

	// Negative test cases
	_, err = tokenAuthService.GenerateNodeToken(nil, notBeforeTS, notAfterTS)
	AssertError(t, err, "GenerateNodeToken should fail when Audience slice is nil")

	_, err = tokenAuthService.GenerateNodeToken([]string{}, notBeforeTS, notAfterTS)
	AssertError(t, err, "GenerateNodeToken should fail when Audience slice is empty")

	_, err = tokenAuthService.GenerateNodeToken([]string{""}, notBeforeTS, notAfterTS)
	AssertError(t, err, "GenerateNodeToken should fail when Audience entry is an empty string")

	_, err = tokenAuthService.GenerateNodeToken([]string{"*", ""}, notBeforeTS, notAfterTS)
	AssertError(t, err, "GenerateNodeToken should fail when one of the Audience entries is an empty string")
}
