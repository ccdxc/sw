// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package state

import (
	"bytes"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/tls"
	"encoding/json"
	"encoding/pem"
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"
	"time"

	"github.com/pensando/sw/nic/agent/nmd/utils"
	"github.com/pensando/sw/nic/agent/protos/nmd"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/revproxy"
)

const (
	minTLSVersion = tls.VersionTLS12
)

var revProxyConfig = map[string]string{
	// NMD
	"/api/v1/naples": "http://127.0.0.1:" + globals.NmdRESTPort,
	"/monitoring/":   "http://127.0.0.1:" + globals.NmdRESTPort,
	"/cores/":        "http://127.0.0.1:" + globals.NmdRESTPort,
	"/cmd/":          "http://127.0.0.1:" + globals.NmdRESTPort,
	"/update/":       "http://127.0.0.1:" + globals.NmdRESTPort,
	"/data/":         "http://127.0.0.1:" + globals.NmdRESTPort,

	// TM-AGENT
	"/telemetry/":           "http://127.0.0.1:" + globals.TmAGENTRestPort,
	"/api/telemetry/fwlog/": "http://127.0.0.1:" + globals.TmAGENTRestPort,

	// EVENTS
	"/api/eventpolicies/": "http://127.0.0.1:" + globals.EvtsProxyRESTPort,

	// NET-AGENT
	"/api/telemetry/flowexports/": "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/networks/":              "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/endpoints/":             "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/sgs/":                   "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/tenants/":               "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/interfaces/":            "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/namespaces/":            "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/nat/pools/":             "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/nat/policies/":          "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/routes/":                "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/nat/bindings/":          "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/ipsec/policies/":        "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/ipsec/encryption/":      "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/ipsec/decryption/":      "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/security/policies/":     "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/security/profiles/":     "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/tunnels/":               "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/tcp/proxies/":           "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/system/ports":           "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/apps":                   "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/vrfs":                   "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/mirror/sessions/":       "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/system/info":            "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/system/debug":           "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/mode/":                  "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/profiles/":              "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/collectors/":            "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/route-tables/":          "http://127.0.0.1:" + globals.AgentRESTPort,
	"/api/routingconfigs/":        "http://127.0.0.1:" + globals.AgentRESTPort,

	"/api ": "http://127.0.0.1:" + globals.AgentRESTPort,

	// Techsupport
	"/api/techsupport/": "http://127.0.0.1:" + globals.NaplesTechSupportRestPort,
	"/api/diagnostics/": "http://127.0.0.1:" + globals.NaplesDiagnosticsRestPort,
}

var protectedCommands = []string{
	"setsshauthkey",
	"penrmauthkeys",
	"penrmsshdfiles",
	"penrmpubkey",
	"mksshdir",
	"touchsshauthkeys",
	"touchsshdlock",
	"enablesshd",
	"startsshd",
}

func authorizeProtectedCommands(req *http.Request) error {
	// We want to make sure that we authorize only protected commands but
	// also that we don't get fooled by malformed requests.

	// Per Go http docs, Empty Method is ok and it means "GET"

	if req.URL == nil {
		log.Errorf("Error authorizing request, empty URL")
		return fmt.Errorf("Error authorizing request, empty URL")
	}

	// if it is not a command, nothing to authorize
	if !strings.HasPrefix(req.URL.Path, "/cmd") {
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

	for _, pc := range protectedCommands {
		if cmdReq.Executable == pc && (req.TLS == nil || len(req.TLS.VerifiedChains) == 0) {
			log.Errorf("Authorization failed: command %s requires authorization token", pc)
			return fmt.Errorf("Authorization failed: command %s requires authorization token", pc)
		}
	}
	return nil
}

func getRevProxyTLSConfig() (*tls.Config, revproxy.Authorizer, error) {
	var clientAuth tls.ClientAuthType
	var authz revproxy.Authorizer

	trustRoots, isClusterRoots, err := utils.GetNaplesTrustRoots()
	if isClusterRoots {
		// trust roots are Venice cluster trust roots, client cert is mandatory, no need to further authorize
		clientAuth = tls.RequireAndVerifyClientCert
	} else {
		// trust roots are Pensando support CA trust roots, client cert is optional, authorize protected commands
		clientAuth = tls.VerifyClientCertIfGiven
		authz = authorizeProtectedCommands
	}

	if err != nil {
		return nil, nil, fmt.Errorf("Error opening trust roots, err: %v", err)
	}

	// use a self-signed certificate for the server so we don't have dependencies on Venice
	privateKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		return nil, nil, fmt.Errorf("Error generating private key. Err: %v", err)
	}
	cert, err := certs.SelfSign("", privateKey, certs.WithNotBefore(certs.BeginningOfTime), certs.WithNotAfter(certs.EndOfTime))
	if err != nil {
		return nil, nil, fmt.Errorf("Error generating self-signed certificate. Err: %v", err)
	}
	log.Infof("Loaded %d trust roots", len(trustRoots))
	for _, c := range trustRoots {
		pemBlock := &pem.Block{
			Type:  certs.CertificatePemBlockType,
			Bytes: c.Raw,
		}
		cb := pem.EncodeToMemory(pemBlock)
		log.Infof("\n%s\n", string(cb))
	}

	return &tls.Config{
		MinVersion:             minTLSVersion,
		SessionTicketsDisabled: true,
		ClientAuth:             clientAuth,
		ClientCAs:              certs.NewCertPool(trustRoots),
		Certificates: []tls.Certificate{
			{
				Certificate: [][]byte{cert.Raw},
				PrivateKey:  privateKey,
			},
		},
	}, authz, nil
}

// StartReverseProxy starts the reverse proxy for all NAPLES REST APIs
func (n *NMD) StartReverseProxy() error {
	// if we have persisted root of trust, require client auth
	tlsConfig, authz, err := getRevProxyTLSConfig()
	if err != nil {
		log.Errorf("Error getting TLS config for reverse proxy: %v", err)
	}
	if tlsConfig == nil {
		log.Infof("NAPLES trust roots not found")
	}
	return n.revProxy.Start(tlsConfig, authz)
}

// StopReverseProxy stops the NMD reverse proxy
func (n *NMD) StopReverseProxy() error {
	return n.revProxy.Stop()
}

// RestartRevProxyWithRetries restarts NMD reverse proxy
func (n *NMD) RestartRevProxyWithRetries() error {
	ticker := time.NewTicker(1 * time.Second)
	defer ticker.Stop()
	done := time.After(20 * time.Second)

	for {
		select {
		case <-ticker.C:
			err := n.StopReverseProxy()
			if err != nil {
				log.Errorf("Failed to stop reverse proxy. Err: %v", err)
			}

			err = n.StartReverseProxy()
			if err != nil {
				log.Errorf("Failed to start reverse proxy. Err : %v", err)
				break
			}

			log.Info("Successfully restarted reverse proxy.")
			return nil
		case <-done:
			log.Error("Failed to restart reverse proxy after 20 seconds.")
			return fmt.Errorf("failed to restart reverse proxy")
		}
	}
}

// UpdateReverseProxyConfig updates the configuration of the NMD reverse proxy
func (n *NMD) UpdateReverseProxyConfig(config map[string]string) error {
	return n.revProxy.UpdateConfig(config)
}
