// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package utils

import (
	"crypto/x509"
	"fmt"
	"os"

	nmdcerts "github.com/pensando/sw/nic/agent/nmd/certs"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"
)

// GetVeniceTrustRoots returns the trust roots that have been persistified on NAPLES
// during the last succesful admission
func GetVeniceTrustRoots() ([]*x509.Certificate, error) {
	_, err := os.Stat(globals.VeniceTrustRootsFile)
	if os.IsNotExist(err) {
		// If a clusterTrustRoots.pem is not found in /sysconfig/config0 try to restrore it from /sysconfig/config1
		err = CheckAndRestoreTrustCerts()
	}

	switch {
	case err == nil:
		// trust roots file is present and opened successfully, validate signature
		trustRoots, err := certs.ReadCertificates(globals.VeniceTrustRootsFile)
		if err != nil {
			return nil, fmt.Errorf("Error reading trust roots file: %v", err)
		}
		return trustRoots, nil
	case os.IsNotExist(err):
		// trust roots file is not present, return nil but no error
		return nil, nil
	default:
		// trust roots file is present but there was an error opening it
		return nil, fmt.Errorf("Error opening trust roots file: %v", err)
	}
}

// ClearVeniceTrustRoots removes persistified trust roots from NAPLES
func ClearVeniceTrustRoots() error {
	err := os.RemoveAll(globals.VeniceTrustRootsFile)
	if err != nil {
		return err
	}

	return os.RemoveAll(globals.VeniceTrustRootsBackupFile)
}

// StoreVeniceTrustRoots updates persistified trust roots on NAPLES
func StoreVeniceTrustRoots(trustRoots []*x509.Certificate) error {
	err := certs.SaveCertificates(globals.VeniceTrustRootsFile, trustRoots)
	if err != nil {
		return err
	}

	return BackupTrustCerts()
}

// GetSupportCATrustRoots returns the trust roots used to validate client certificates when accessing protected resources on NAPLES REST API
func GetSupportCATrustRoots() ([]*x509.Certificate, error) {
	return certs.DecodePEMCertificates([]byte(nmdcerts.SupportCATrustRoots))
}

// GetNaplesTrustRoots looks for Venice trust roots that have been persistified on NAPLES
// during the last succesful admission. If it doesn't find any, it defaults to SupportCATrustRoots.
// The second parameter returns whether these are cluster trust roots or Pensando trust roots.
func GetNaplesTrustRoots() ([]*x509.Certificate, bool, error) {
	tr, err := GetVeniceTrustRoots()
	if tr != nil {
		return tr, true, nil
	}
	log.Infof("Unable to open load Venice trust roots, err: %v", err)

	tr, err = GetSupportCATrustRoots()
	if tr != nil {
		return tr, false, nil
	}
	log.Infof("Unable to open load SupportCA trust roots, err: %v", err)

	return nil, false, err
}
