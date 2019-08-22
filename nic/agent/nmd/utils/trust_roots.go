// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package utils

import (
	"crypto/x509"
	"fmt"
	"os"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
)

// GetNaplesTrustRoots returns the trust roots that have been persistified on NAPLES
// during the last succesful admission
func GetNaplesTrustRoots() ([]*x509.Certificate, error) {
	_, err := os.Stat(globals.NaplesTrustRootsFile)
	if os.IsNotExist(err) {
		// If a clusterTrustRoots.pem is not found in /sysconfig/config0 try to restrore it from /sysconfig/config1
		err = CheckAndRestoreTrustCerts()
	}

	switch {
	case err == nil:
		// trust roots file is present and opened successfully, validate signature
		trustRoots, err := certs.ReadCertificates(globals.NaplesTrustRootsFile)
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

// ClearNaplesTrustRoots removes persistified trust roots from NAPLES
func ClearNaplesTrustRoots() error {
	err := os.RemoveAll(globals.NaplesTrustRootsFile)
	if err != nil {
		return nil
	}

	return os.RemoveAll(globals.NaplesTrustRootsBackupFile)
}

// StoreTrustRoots updates persistified trust roots on NAPLES
func StoreTrustRoots(trustRoots []*x509.Certificate) error {
	err := certs.SaveCertificates(globals.NaplesTrustRootsFile, trustRoots)
	if err != nil {
		return err
	}

	return BackupTrustCerts()
}
