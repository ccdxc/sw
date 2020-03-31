package state

import (
	"bytes"
	"crypto/x509"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/protos/nmd"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/revproxy"
)

const (
	commandURLPrefix = "/cmd"
	dscURISchema     = "dsc"
)

// AudienceExtractor is a pluggable function that extracts audience information embedded in a certificate
type AudienceExtractor func(cert *x509.Certificate) ([]string, error)

// ExtractAudienceFromCertForHostManagedDSC extracts audience from a cert issued by Pensando for clients to interact directly with DSC in host managed mode
func ExtractAudienceFromCertForHostManagedDSC(cert *x509.Certificate) ([]string, error) {
	var audience []string
	for _, uri := range cert.URIs {
		switch uri.Scheme {
		case dscURISchema:
			audience = append(audience, uri.Host)
		default:
			log.Errorf("Found unknown scheme '%s' in URI: %v", uri.Scheme, uri)
		}
	}
	return audience, nil
}

// MacBasedAuthorizer checks for device specific mac-address within client provided token/certificate
type MacBasedAuthorizer struct {
	macAddr           string
	audienceExtractor AudienceExtractor
}

// Authorize checks for device specific mac-address within client provided token/certificate depending on isClientCertRequired flag
func (ia *MacBasedAuthorizer) Authorize(req *http.Request) error {
	audience, err := ia.GetAudience(req)
	if err != nil {
		log.Errorf("Unable to extract audience from client certificate: %s", err.Error())
		return errors.Wrap(err, "Authorization failed, unable to extract audience from client certificate")
	}

	if len(audience) == 0 {
		log.Errorf("No audience found in the client certificate")
		return errors.New("Authorization failed, no audience found in the client certificate")
	}

	for _, permittedEntity := range audience {
		if permittedEntity == "*" || strings.EqualFold(permittedEntity, ia.macAddr) {
			return nil
		}
	}
	log.Errorf("client is not provisioned to access device with mac: %s. ", ia.macAddr)
	return errors.Errorf("Unauthorized access, client is not provisioned to access device with mac: %s. ", ia.macAddr)
}

// GetAudience extracts audience from TLS state recorded by http server within the http.Request provided as argument.
// Note that this can only be used on the server side, invoking this utility on client side http.Request object is incorrect.
func (ia *MacBasedAuthorizer) GetAudience(request *http.Request) ([]string, error) {
	if request.TLS == nil || request.TLS.VerifiedChains == nil || len(request.TLS.VerifiedChains) == 0 {
		return nil, errors.New("No verified client certificate found")
	}
	verifiedCert := request.TLS.VerifiedChains[0][0]
	return ia.audienceExtractor(verifiedCert)
}

// ProtectedCommandsAuthorizer checks the request for specific 'protected' commands is made using valid token/certificate
type ProtectedCommandsAuthorizer struct {
	chainedAuthorizer revproxy.Authorizer
	protectedCommands []string
}

// Authorize checks the request for protectedCommands is made using valid token/certificate
func (pca *ProtectedCommandsAuthorizer) Authorize(req *http.Request) error {
	// We want to make sure that we authorize only protected commands but
	// also that we don't get fooled by malformed requests.

	// Per Go http docs, Empty Method is ok and it means "GET"

	if req.URL == nil {
		log.Errorf("Error authorizing request, empty URL")
		return fmt.Errorf("Error authorizing request, empty URL")
	}

	// if it is not a command, nothing to authorize
	if !strings.HasPrefix(req.URL.Path, commandURLPrefix) {
		return nil
	}

	if req.Body == nil {
		log.Errorf("Error authorizing request, empty body")
		return fmt.Errorf("Error authorizing cmd request, empty body")
	}

	var bodyBytes []byte
	// if we weren't able to read the entire req, better to bail out and let client retry
	bodyBytes, err := ioutil.ReadAll(req.Body)
	if err != nil {
		log.Errorf("Error authorizing request %s %s: %v", req.Method, req.URL, err)
		return fmt.Errorf("Error authorizing request: %v", err)
	}

	// Restore the io.ReadCloser to its original state, otherwise the handler won't be able to read it
	req.Body = ioutil.NopCloser(bytes.NewBuffer(bodyBytes))

	cmdReq := nmd.DistributedServiceCardCmdExecute{}
	err = json.Unmarshal(bodyBytes, &cmdReq)
	if err != nil {
		log.Errorf("Error authorizing request %s %s: %v", req.Method, req.URL, err)
		return fmt.Errorf("Error authorizing request: %v", err)
	}

	for _, pc := range pca.protectedCommands {
		if cmdReq.Executable == pc {
			if req.TLS == nil || len(req.TLS.VerifiedChains) == 0 {
				log.Errorf("Authorization failed: command %s requires authorization token", pc)
				return fmt.Errorf("Authorization failed: command %s requires authorization token", pc)
			}
			// invoke chained authorizer only for protected commands
			return pca.chainedAuthorizer.Authorize(req)
		}
	}
	return nil
}
