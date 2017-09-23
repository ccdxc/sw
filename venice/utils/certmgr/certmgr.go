// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package certmgr

import (
	"fmt"
	"os"
	"path"

	"github.com/pkg/errors"
)

// CertificateMgr is the service for storing, issuing and rotating certificates
type CertificateMgr struct {
	workDir string                // The working directory for the instance
	ca      *CertificateAuthority // Internal certificate authority
	ready   bool
}

// NewCertificateMgr provides a new instance of Certificate Manager
func NewCertificateMgr(dir string) (*CertificateMgr, error) {
	if dir == "" {
		return nil, errors.New("CertificateMgr root directory is required")
	}

	err := os.MkdirAll(dir, 0700)
	if err != nil {
		return nil, errors.Wrap(err, fmt.Sprintf("Could not open root directory, path: %v", dir))
	}

	svc := &CertificateMgr{
		workDir: dir,
		ready:   false,
	}

	ca, err := NewCertificateAuthority(path.Join(dir, "CA"))
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
