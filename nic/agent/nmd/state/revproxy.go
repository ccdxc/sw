// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package state

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/tls"
	"encoding/pem"
	"fmt"
	"time"

	"github.com/pensando/sw/nic/agent/nmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"
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

	"/api ": "http://127.0.0.1:" + globals.AgentRESTPort,

	// Techsupport
	"/api/techsupport/": "http://127.0.0.1:" + globals.NaplesTechSupportRestPort,
	"/api/diagnostics/": "http://127.0.0.1:" + globals.NaplesDiagnosticsRestPort,
}

func getRevProxyTLSConfig() (*tls.Config, error) {
	trustRoots, err := utils.GetNaplesTrustRoots()
	if trustRoots != nil {
		// use a self-signed certificate for the server so we don't have dependencies on Venice
		privateKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
		if err != nil {
			return nil, fmt.Errorf("Error generating private key. Err: %v", err)
		}
		cert, err := certs.SelfSign("", privateKey, certs.WithNotBefore(certs.BeginningOfTime), certs.WithNotAfter(certs.EndOfTime))
		if err != nil {
			return nil, fmt.Errorf("Error generating self-signed certificate. Err: %v", err)
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
			ClientAuth:             tls.RequireAndVerifyClientCert,
			ClientCAs:              certs.NewCertPool(trustRoots),
			Certificates: []tls.Certificate{
				{
					Certificate: [][]byte{cert.Raw},
					PrivateKey:  privateKey,
				},
			},
		}, nil
	}
	if err != nil {
		return nil, fmt.Errorf("Error getting NAPLES trust roots: %v", err)
	}
	return nil, nil
}

// StartReverseProxy starts the reverse proxy for all NAPLES REST APIs
func (n *NMD) StartReverseProxy() error {
	// if we have persisted root of trust, require client auth
	tlsConfig, err := getRevProxyTLSConfig()
	if err != nil {
		log.Errorf("Error getting TLS config for reverse proxy: %v", err)
	}
	if tlsConfig == nil {
		log.Infof("NAPLES trust roots not found")
	}
	return n.revProxy.Start(tlsConfig)
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
