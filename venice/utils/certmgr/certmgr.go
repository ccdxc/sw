// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package certmgr

import (
	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/keymgr"
)

// CertificateMgr is the service for storing, issuing and rotating certificates
type CertificateMgr struct {
	ca    *CertificateAuthority // Internal certificate authority
	ready bool
}

// NewCertificateMgr provides a new instance of Certificate Manager
func NewCertificateMgr(km *keymgr.KeyMgr) (*CertificateMgr, error) {
	if km == nil {
		return nil, errors.New("KeyMgr instance is required")
	}

	svc := &CertificateMgr{
		ready: false,
	}

	ca, err := NewCertificateAuthority(km)
	if err != nil || !ca.IsReady() {
		return nil, err
	}

	svc.ca = ca
	svc.ready = true

	return svc, nil
}

// Ca returns a handle to the CA that is used by Certificate Mgr to issue certificates
func (cm *CertificateMgr) Ca() *CertificateAuthority {
	return cm.ca
}

// IsReady returns true when instance is ready to serve requests
func (cm *CertificateMgr) IsReady() bool {
	return cm.ready
}
