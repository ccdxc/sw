package readutils

import (
	"crypto/x509"
	"encoding/pem"
	"errors"
	"fmt"
	"strings"
	"testing"

	"github.com/pensando/sw/venice/utils/certs"
	. "github.com/pensando/sw/venice/utils/testutils"
	. "github.com/pensando/sw/venice/utils/testutils/certs"
)

const (
	wildcardAudience = "*"
	testMacAddress   = "test_mac_addr"
)

const (
	expectedTokenAudience = "random.test.mac"
	veniceToken           = `-----BEGIN CERTIFICATE-----
MIIDlTCCAX2gAwIBAgIQbWbOtfzkkjdxWJBf3oDekzANBgkqhkiG9w0BAQsFADAU
MRIwEAYDVQQDEwlDTURSb290Q0EwIBcNNzAwMTAxMDAwMDAwWhgPOTk5OTEyMzEy
MzU5MDBaMAAwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAASwt2/ptAs1DS0anHMyHgcG
8Dnp4cH5sX38uv/C5DVk2STHp+ASf2P1Wr0//Kz/pPifv7vY43OtLgW+fitLYeSC
//OtUrHCSQ7flpNTL5OuIQEv0MWGUBHQZVuEimGGSrSjgaIwgZ8wDgYDVR0PAQH/
BAQDAgeAMBMGA1UdJQQMMAoGCCsGAQUFBwMCMAwGA1UdEwEB/wQCMAAwHQYDVR0O
BBYEFMWs78YLq2eB9r69C2A9UMdw+dMoMA4GA1UdIwQHMAWAAwECAzA7BgNVHREB
Af8EMTAvhi12ZW5pY2U6Ly90ZXN0Q2x1c3Rlci9hdWRpZW5jZT9yYW5kb20udGVz
dC5tYWMwDQYJKoZIhvcNAQELBQADggIBABsA5dfbrpxLiqhoiuO8Ft8400JiKeS3
MjLnO+zpvaof2UqOM4hdhxKnTG8j8Aat1lcDms0HpV0tQZgeaC9IMrWgSgF17wig
g0eazIAQ+IhcfI4vbC1BXgLnYSLc9mEFtu+qMPWO8gPsqTF7BUe8yxfko+CYq6MY
ayE4IxgZ+JnONYRao2ASWJTob1M8whEqXarMn8kiQMMoS45u/6snAks1BQFd49hO
oKsHP/JTrUCcBrGiICXkZDUIYfG32os9F5CoocRW0y9rBlYFsLqXVpTQqVHASOd3
hcGY+ilazIfn+/hNX0AGRbpnS/+ylshzW+/iE9iIoGmJzy/eC49sRQ8kO98ez2cO
eDqDouk3TWGPXBCOZOIA6lkwT6KGEGagdcioT/tcrwxjZDh4WL1SZnNPsMkWCTWM
6c0Zam4WmxpFS2Ud9jQ09SRCiHyP5wUBxrM2Bqo657E9g+uDqPbCyc4So6W/asir
g1bsKawb57orB5bgrFNeUak1UPW+MVMUV/7ntWpmF7Ys0r+7NAaKXnbYUaA7G/yF
kwr3/+Au9mwYVa5oy8osps7CkQKNgBD2lnoDKQnAERiLh1xHHEuwycuaiscPaDii
+GRUCJofhVuzQYYqDmR9AqNuURxi8bgw43mix/dII7i6VvVJw2P6gvcaAVfyloWz
ipwtGQysUQ58
-----END CERTIFICATE-----
-----BEGIN PRIVATE KEY-----
MIG2AgEAMBAGByqGSM49AgEGBSuBBAAiBIGeMIGbAgEBBDD8OaIuSNqwuEcETgdp
kBLXTtp/MZUpfMddqDUmgv7d1vVML/FJSq9Q1HKC8mn3B4uhZANiAASwt2/ptAs1
DS0anHMyHgcG8Dnp4cH5sX38uv/C5DVk2STHp+ASf2P1Wr0//Kz/pPifv7vY43Ot
LgW+fitLYeSC//OtUrHCSQ7flpNTL5OuIQEv0MWGUBHQZVuEimGGSrQ=
-----END PRIVATE KEY-----`
)

func TestGetNodeTokenAttributes(t *testing.T) {

	actualAudience, err := GetNodeTokenAttributes(veniceToken)
	AssertOk(t, err, "GetNodeTokenAttributes could not parse tokenString: %s", veniceToken)
	AssertEquals(t, []string{expectedTokenAudience}, actualAudience, "GetNodeTokenAttributes did not return expected audience")

	// negative testcases
	_, err = GetNodeTokenAttributes("")
	AssertError(t, err, "GetNodeTokenAttributes did not return error")
	_, err = GetNodeTokenAttributes("Hello")
	AssertError(t, err, "GetNodeTokenAttributes did not return error")

	_, privateKey, err := GetVeniceIssuedTokenContents(t, []string{"foo"})
	AssertOk(t, err, "Error generating node token")

	p8key, err := x509.MarshalPKCS8PrivateKey(privateKey)
	AssertOk(t, err, "Error marshaling PKCS8 private key: %v", err)

	var tokenWithoutCert []byte
	block := &pem.Block{
		Type:  certs.PrivateKeyPemBlockType,
		Bytes: p8key,
	}
	tokenWithoutCert = pem.EncodeToMemory(block)
	_, err = GetNodeTokenAttributes(string(tokenWithoutCert))
	AssertError(t, err, "GetNodeTokenAttributes did not return error")
}

func TestAudienceExtractionFromClientCert(t *testing.T) {
	testClientCert, err := GetVeniceIssuedClientCertificate(t, []string{wildcardAudience})
	AssertOk(t, err, "Unable to create test client cert")
	audience, err := ExtractAudienceFromVeniceCert(testClientCert)
	AssertOk(t, err, "Audience extractor failed to get audience from Venice issued client certificate")
	Assert(t, len(audience) == 1, "Expected to find a single entry in Audience slice")
	Assert(t, audience[0] == wildcardAudience, "Expected to find '%s' entry in Audience", wildcardAudience)

	testClientCert, err = GetVeniceIssuedClientCertificate(t, []string{testMacAddress})
	AssertOk(t, err, "Unable to create test client cert")
	audience, err = ExtractAudienceFromVeniceCert(testClientCert)
	AssertOk(t, err, "Audience extractor failed to get audience from Venice issued client certificate")
	Assert(t, len(audience) == 1, "Expected to find a single entry in Audience slice")
	Assert(t, audience[0] == testMacAddress, "Expected to find '%s' entry in Audience", testMacAddress)

	testClientCert, err = getClientCertificateWithInvalidScheme(t, []string{wildcardAudience})
	AssertOk(t, err, "Unable to create test client cert")
	audience, err = ExtractAudienceFromVeniceCert(testClientCert)
	AssertOk(t, err, "Audience extractor failed to get audience from Venice issued client certificate")
	Assert(t, len(audience) == 0, "Expected to find an Audience slice")

	testClientCert, err = getClientCertificateWithInvalidPath(t, []string{wildcardAudience})
	AssertOk(t, err, "Unable to create test client cert")
	audience, err = ExtractAudienceFromVeniceCert(testClientCert)
	AssertOk(t, err, "Audience extractor failed to get audience from Venice issued client certificate")
	Assert(t, len(audience) == 0, "Expected to find an Audience slice")
}

func getClientCertificateWithInvalidScheme(t *testing.T, audience []string) (*x509.Certificate, error) {
	if len(audience) == 0 {
		return nil, errors.New("Can not generate a test certificate for empty audience")
	}
	audienceURIStr := fmt.Sprintf("wrongscheme://%s", strings.Join(audience, "&"))
	return GetClientCertificate(t, audienceURIStr)
}

func getClientCertificateWithInvalidPath(t *testing.T, audience []string) (*x509.Certificate, error) {
	if len(audience) == 0 {
		return nil, errors.New("Can not generate a test certificate for empty audience")
	}
	audienceURIStr := fmt.Sprintf("venice://%s/%s?%s", "test_cluster", "wrongPath", strings.Join(audience, "&"))
	return GetClientCertificate(t, audienceURIStr)
}
