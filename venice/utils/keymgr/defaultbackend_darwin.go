// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package keymgr

import (
	"io/ioutil"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/log"
)

// NewDefaultBackend returns an instance of the default backend
// It allows switching from one backend to the other without making clients of KeyMgr aware of the change
func NewDefaultBackend(ID string) (Backend, error) {
	// GoCrypto is only available choice on Mac
	workDir, err := ioutil.TempDir("", "gocryptobe-"+ID+"-")
	if err != nil {
		return nil, errors.Wrapf(err, "Error creating workdir for GoCrypto backend")
	}
	log.Infof("Default backend is GoCryptoBackend, workDir: %v", workDir)
	return NewGoCryptoBackend(workDir)
}
