// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package tokenauth

import (
	"fmt"
	"time"

	"github.com/pensando/sw/venice/utils/tokenauth"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/certmgr"
	"github.com/pensando/sw/venice/utils/certs"
)

// TokenAuth represents an instance of the TokenAuth service
type TokenAuth struct {
	clusterName string
	ca          *certmgr.CertificateAuthority
}

// NewTokenAuthService returns a new instance of the TokenAuth service
func NewTokenAuthService(clusterName string, ca *certmgr.CertificateAuthority) *TokenAuth {
	return &TokenAuth{
		clusterName: clusterName,
		ca:          ca,
	}
}

// GenerateNodeToken generates a node token for audience
func (n *TokenAuth) GenerateNodeToken(audience []string, validityStart, validityEnd *api.Timestamp) (string, error) {
	if !n.ca.IsReady() {
		return "", fmt.Errorf("CA not ready")
	}
	if audience == nil || len(audience) == 0 {
		return "", fmt.Errorf("Audience must be specified")
	}
	for _, a := range audience {
		if a == "" {
			return "", fmt.Errorf("Audience entry can not be an empty string")
		}
	}
	var err error
	var notBefore, notAfter time.Time
	if validityStart != nil {
		notBefore, err = validityStart.Time()
		if err != nil {
			return "", fmt.Errorf("Invalid validityStart time: %v", validityStart)
		}
	} else {
		notBefore = certs.BeginningOfTime
	}
	if validityEnd != nil {
		notAfter, err = validityEnd.Time()
		if err != nil {
			return "", fmt.Errorf("Invalid validityEnd time: %v", validityEnd)
		}
	} else {
		notAfter = certs.EndOfTime
	}

	return tokenauth.MakeNodeToken(n.ca, n.clusterName, audience, notBefore, notAfter)
}
