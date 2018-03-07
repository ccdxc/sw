// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// +build linux,!arm64

package keymgr

import (
	"io/ioutil"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/log"
)

// NewDefaultBackend returns an instance of the default backend
// It allows switching from one backend to the other without making clients of KeyMgr aware of the change
func NewDefaultBackend(ID string) (Backend, error) {
	// On linux we use SoftHSM backend if available
	pkcs11be, err := NewPkcs11Backend(Pkcs11Lib, "pkcs11be-"+ID, "1234")
	if err == nil {
		hsmInfo, _ := pkcs11be.GetInfo() // do not return failure if the call to GetInfo fails
		log.Infof("Default backend is Pkcs11Backend, ID: %v, module: %v, moduleInfo:%+v", ID, Pkcs11Lib, hsmInfo)
		return pkcs11be, nil
	}
	// else fall back to GoCrypto backend
	log.Infof("PKCS11 backend unavailable: %v", err)
	workDir, err := ioutil.TempDir("", "gocryptobe-"+ID)
	if err != nil {
		return nil, errors.Wrapf(err, "Error creating workdir for GoCrypto backend")
	}
	log.Infof("Default backend is GoCryptoBackend, workDir: %v", workDir)
	return NewGoCryptoBackend(workDir)
}
