package state

import (
	"bytes"
	"crypto/tls"
	"crypto/x509"
	"encoding/json"
	"errors"
	"fmt"
	"net/http"
	"net/url"
	"strings"
	"testing"

	. "github.com/pensando/sw/venice/utils/tokenauth/readutils"

	"github.com/golang/mock/gomock"

	mock_revproxy "github.com/pensando/sw/venice/utils/revproxy/mock"

	"github.com/pensando/sw/nic/agent/protos/nmd"
	. "github.com/pensando/sw/venice/utils/testutils"
	. "github.com/pensando/sw/venice/utils/testutils/certs"
)

const (
	wildcardAudience    = "*"
	testMacAddress      = "test_mac_addr"
	differentMacAddress = "different_mac_addr"
	protectedCommand    = "enablesshd"
	unprotectedCommand  = "consoleenable"
)

func TestAudienceExtractor_ForHostManagedDSCCerts(t *testing.T) {
	testClientCert, err := getTestPensandoIssuedClientCertificate(t, []string{wildcardAudience})
	AssertOk(t, err, "Unable to create test client cert")
	audience, err := ExtractAudienceFromCertForHostManagedDSC(testClientCert)
	AssertOk(t, err, "Audience extractor failed to get audience from Pensando issued client certificate")
	Assert(t, len(audience) == 1, "Expected to find a single entry in Audience slice")
	Assert(t, audience[0] == wildcardAudience, "Expected to find '%s' entry in Audience", wildcardAudience)

	testClientCert, err = getTestPensandoIssuedClientCertificate(t, []string{testMacAddress})
	AssertOk(t, err, "Unable to create test client cert")
	audience, err = ExtractAudienceFromCertForHostManagedDSC(testClientCert)
	AssertOk(t, err, "Audience extractor failed to get audience from Pensando issued client certificate")
	Assert(t, len(audience) == 1, "Expected to find a single entry in Audience slice")
	Assert(t, audience[0] == testMacAddress, "Expected to find '%s' entry in Audience", testMacAddress)

	testClientCert, err = getTestClientCertificateWithInvalidScheme(t, []string{wildcardAudience})
	AssertOk(t, err, "Unable to create test client cert")
	audience, err = ExtractAudienceFromCertForHostManagedDSC(testClientCert)
	AssertOk(t, err, "Audience extractor failed to get audience from Pensando issued client certificate")
	Assert(t, len(audience) == 0, "Expected to find an empty Audience slice")
}

func TestMacBasedAuthorizer_Authorize(t *testing.T) {
	testRequest := httpRequestWithoutClientCertificate()

	authorizerToTest := &MacBasedAuthorizer{
		macAddr:           testMacAddress,
		audienceExtractor: ExtractAudienceFromVeniceCert,
	}
	err := authorizerToTest.Authorize(testRequest)
	AssertError(t, err, "Authorizer failed to reject request without a client certificate")

	testRequest = httpRequestWithVerifiedClientCertificate(t, []string{"*"})
	err = authorizerToTest.Authorize(testRequest)
	AssertOk(t, err, "Authorizer failed to allow certificate with wildcard ('*') audience")

	testRequest = httpRequestWithVerifiedClientCertificate(t, []string{testMacAddress})
	err = authorizerToTest.Authorize(testRequest)
	AssertOk(t, err, "Authorizer failed to allow certificate with specific mac_address ('%s') audience", testMacAddress)

	testRequest = httpRequestWithVerifiedClientCertificate(t, []string{strings.ToUpper(testMacAddress)})
	err = authorizerToTest.Authorize(testRequest)
	AssertOk(t, err, "Authorizer failed to allow certificate with same mac_address ('%s') audience, but different case", testMacAddress)

	testRequest = httpRequestWithVerifiedClientCertificate(t, []string{differentMacAddress})
	err = authorizerToTest.Authorize(testRequest)
	AssertError(t, err, "Authorizer failed to reject certificate with different mac_address ('%s') audience from the host", differentMacAddress)

	testRequest = httpRequestWithInvalidClientCertificate(t, []string{testMacAddress})
	err = authorizerToTest.Authorize(testRequest)
	AssertError(t, err, "Authorizer failed to reject certificate with invalid URI")
}

func TestProtectedCommandsAuthorizer_Authorize(t *testing.T) {
	protectedCommandHTTPRequest := getHTTPRequest(t, protectedCommand)

	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()

	mockAuthorizer := mock_revproxy.NewMockAuthorizer(mockCtrl)

	protectedCommandsAuthorizer := &ProtectedCommandsAuthorizer{
		chainedAuthorizer: mockAuthorizer,
		protectedCommands: []string{protectedCommand},
	}
	err := protectedCommandsAuthorizer.Authorize(protectedCommandHTTPRequest)
	AssertError(t, err, "Authorizer failed to reject command execution request with no verified client certificates")

	//now set the TLS state as verified
	setTLSStateAsVerified(t, protectedCommandHTTPRequest)
	mockAuthorizer.EXPECT().Authorize(gomock.Eq(protectedCommandHTTPRequest)).Return(nil).Times(1)
	err = protectedCommandsAuthorizer.Authorize(protectedCommandHTTPRequest)
	AssertOk(t, err, "Authorizer failed to allow protected command with verified client certificate and when chained authorizers allows the request")

	mockAuthorizer.EXPECT().Authorize(gomock.Eq(protectedCommandHTTPRequest)).Return(errors.New("chained auth failed")).Times(1)
	err = protectedCommandsAuthorizer.Authorize(protectedCommandHTTPRequest)
	AssertError(t, err, "Authorizer failed to reject protected command when chained authorizer rejected the request")

	notProtectedCommandHTTPRequest := getHTTPRequest(t, unprotectedCommand)
	//chained authorizer should not be invoked for unprotected commands
	mockAuthorizer.EXPECT().Authorize(gomock.Eq(notProtectedCommandHTTPRequest)).Return(nil).Times(0)
	err = protectedCommandsAuthorizer.Authorize(notProtectedCommandHTTPRequest)
	AssertOk(t, err, "Authorizer failed to allow not-protected command without any verified client certificate")

	setTLSStateAsVerified(t, notProtectedCommandHTTPRequest)
	//chained authorizer should not be invoked for unprotected commands
	mockAuthorizer.EXPECT().Authorize(gomock.Eq(notProtectedCommandHTTPRequest)).Return(nil).Times(0)
	err = protectedCommandsAuthorizer.Authorize(notProtectedCommandHTTPRequest)
	AssertOk(t, err, "Authorizer failed to allow not-protected command with verified client certificate")

	requestWithNoURL := &http.Request{}
	mockAuthorizer.EXPECT().Authorize(gomock.Eq(requestWithNoURL)).Return(nil).Times(0)
	err = protectedCommandsAuthorizer.Authorize(requestWithNoURL)
	AssertError(t, err, "Authorizer failed to reject request without a URL")

	requestWithNoBody := &http.Request{
		URL: &url.URL{
			Path: "/cmd/v1/naples",
		},
	}
	mockAuthorizer.EXPECT().Authorize(gomock.Eq(requestWithNoBody)).Return(nil).Times(0)
	err = protectedCommandsAuthorizer.Authorize(requestWithNoBody)
	AssertError(t, err, "Authorizer failed to reject request without a Body")

	requestWithMalformedBody := getHTTPRequestWithMalformedBody(t)
	mockAuthorizer.EXPECT().Authorize(gomock.Eq(requestWithMalformedBody)).Return(nil).Times(0)
	err = protectedCommandsAuthorizer.Authorize(requestWithMalformedBody)
	AssertError(t, err, "Authorizer failed to reject request with malformed Body")

	nonCmdRequest := &http.Request{
		URL: &url.URL{
			Path: "/etc/timezone",
		},
	}
	mockAuthorizer.EXPECT().Authorize(gomock.Eq(nonCmdRequest)).Return(nil).Times(0)
	err = protectedCommandsAuthorizer.Authorize(nonCmdRequest)
	AssertOk(t, err, "Authorizer failed to allow a non-cmd request")
}

func setTLSStateAsVerified(t *testing.T, dscExecHTTPRequest *http.Request) {
	dscExecHTTPRequest.TLS = getTLSConnectionStateWithVerifiedChains(t, []string{"*"})
}

func getHTTPRequest(t *testing.T, dscExecutable string) *http.Request {
	dscCommand := &nmd.DistributedServiceCardCmdExecute{
		Executable: dscExecutable,
		Opts:       strings.Join([]string{""}, ""),
	}
	url := "http://127.0.0.1:8888/cmd/v1/naples/"
	payloadBytes, err := json.Marshal(dscCommand)
	AssertOk(t, err, "Error serializing dscCommand: %+v", dscCommand)
	body := bytes.NewReader(payloadBytes)
	getReq, err := http.NewRequest("GET", url, body)
	AssertOk(t, err, "Error creating http request")
	return getReq
}

func getHTTPRequestWithMalformedBody(t *testing.T) *http.Request {
	url := "http://127.0.0.1:8888/cmd/v1/naples/"
	payload := "just another string"
	payloadBytes, err := json.Marshal(payload)
	AssertOk(t, err, "Error serializing dscCommand: %s", payload)
	body := bytes.NewReader(payloadBytes)
	getReq, err := http.NewRequest("GET", url, body)
	AssertOk(t, err, "Error creating http request")
	return getReq
}

func httpRequestWithoutClientCertificate() *http.Request {
	return &http.Request{TLS: &tls.ConnectionState{}}
}

func httpRequestWithVerifiedClientCertificate(t *testing.T, audience []string) *http.Request {
	return &http.Request{TLS: getTLSConnectionStateWithVerifiedChains(t, audience)}
}

func httpRequestWithInvalidClientCertificate(t *testing.T, audience []string) *http.Request {
	clientCert, err := getTestClientCertificateWithInvalidScheme(t, audience)
	AssertOk(t, err, "Error creating test client certificate")
	return &http.Request{
		TLS: &tls.ConnectionState{
			HandshakeComplete: true,
			VerifiedChains:    [][]*x509.Certificate{{clientCert}},
		},
	}
}

func getTLSConnectionStateWithVerifiedChains(t *testing.T, audience []string) *tls.ConnectionState {
	clientCert, err := getTestVeniceIssuedClientCertificate(t, audience)
	AssertOk(t, err, "Error creating test client certificate")
	return &tls.ConnectionState{
		HandshakeComplete: true,
		VerifiedChains:    [][]*x509.Certificate{{clientCert}},
	}
}

func getTestVeniceIssuedClientCertificate(t *testing.T, audience []string) (*x509.Certificate, error) {
	if len(audience) == 0 {
		return nil, errors.New("Can not generate a test certificate for empty audience")
	}
	audienceURIStr := fmt.Sprintf("venice://%s/%s?%s", "test_cluster", "audience", strings.Join(audience, "&"))
	return GetClientCertificate(t, audienceURIStr)
}

func getTestClientCertificateWithInvalidScheme(t *testing.T, audience []string) (*x509.Certificate, error) {
	if len(audience) == 0 {
		return nil, errors.New("Can not generate a test certificate for empty audience")
	}
	audienceURIStr := fmt.Sprintf("wrongscheme://%s", strings.Join(audience, "&"))
	return GetClientCertificate(t, audienceURIStr)
}

func getTestPensandoIssuedClientCertificate(t *testing.T, audience []string) (*x509.Certificate, error) {
	if len(audience) == 0 {
		return nil, errors.New("Can not generate a test certificate for empty audience")
	}
	audienceURIStr := fmt.Sprintf("dsc://%s", strings.Join(audience, "&"))
	return GetClientCertificate(t, audienceURIStr)
}
