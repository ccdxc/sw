// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/log"
)

// NewDefaultBackend returns an instance of the default backend
// It allows switching from one backend to the other without making clients of KeyMgr aware of the change
func NewDefaultBackend() (Backend, error) {
	// On linux we use SoftHSM backend
	pkcs11be, err := NewPkcs11Backend(Pkcs11Lib, "keymgr", "1234")
	if err != nil {
		return nil, errors.Wrapf(err, "Error instantiating default backend: %s", err)
	}
	hsmInfo, _ := pkcs11be.GetInfo() // do not return failure if the call to GetInfo fails
	log.Infof("Default backend is Pkcs11Backend, module: %v, moduleInfo:%+v", Pkcs11Lib, hsmInfo)
	return pkcs11be, nil
}
