// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package configs

import (
	"fmt"

	"github.com/pensando/sw/venice/cmd/credentials"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
)

// GenerateElasticAuthConfig generates configuration file for elastic service
func GenerateElasticAuthConfig(nodeID string) error {
	if env.CertMgr == nil {
		return fmt.Errorf("Failed to generate Elastic auth config, CertMgr not available")
	}
	if !env.CertMgr.IsReady() {
		return fmt.Errorf("Failed to generate Elastic auth config, CertMgr not ready")
	}
	return credentials.GenElasticAuth(nodeID, env.CertMgr.Ca().Sign, env.CertMgr.Ca().TrustRoots())
}

// RemoveElasticAuthConfig removes elastic-discovery config file
func RemoveElasticAuthConfig() {
	certs.DeleteTLSCredentials(globals.ElasticNodeAuthDir)
	certs.DeleteTLSCredentials(globals.ElasticHTTPSAuthDir)
	certs.DeleteTLSCredentials(globals.ElasticClientAuthDir)
}
