// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package platform

import (
	"crypto"
	"sync"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/nmd/state"
)

// NaplesPlatformAgent is the mock platform for testing purposes
type NaplesPlatformAgent struct {
	nmd         state.NmdPlatformAPI // NMD server instance
	certificate []byte               // Factory certificate
	key         crypto.Signer        // private key for factory certificate
	sync.Mutex
}

// NewNaplesPlatformAgent returns a new mock platform agent
func NewNaplesPlatformAgent() (*NaplesPlatformAgent, error) {
	// create new naples platform
	np := NaplesPlatformAgent{}
	return &np, nil
}

// RegisterNMD sets the agent for this platform
func (np *NaplesPlatformAgent) RegisterNMD(nmd state.NmdPlatformAPI) error {
	// store the nmd instance
	np.Lock()
	defer np.Unlock()
	np.nmd = nmd
	return nil
}

// CreateSmartNIC creates a SmartNIC in platform
func (np *NaplesPlatformAgent) CreateSmartNIC(nw *cluster.SmartNIC) error {
	return nil
}

// UpdateSmartNIC updates a SmartNIC in platform
func (np *NaplesPlatformAgent) UpdateSmartNIC(nw *cluster.SmartNIC) error {
	return nil
}

// DeleteSmartNIC deletes a SmartNIC from platform
func (np *NaplesPlatformAgent) DeleteSmartNIC(nw *cluster.SmartNIC) error {
	return nil
}

// GetPlatformCertificate returns the certificate containing the NIC identity and public key
func (np *NaplesPlatformAgent) GetPlatformCertificate(nic *cluster.SmartNIC) ([]byte, error) {
	np.Lock()
	defer np.Unlock()
	if np.certificate == nil {
		np.initPlatformCredentials(nic)
	}
	return np.certificate, nil
}

// GetPlatformSigner returns a handle to the NIC private key that can be used for attestation
func (np *NaplesPlatformAgent) GetPlatformSigner(nic *cluster.SmartNIC) (crypto.Signer, error) {
	np.Lock()
	defer np.Unlock()
	if np.key == nil {
		np.initPlatformCredentials(nic)
	}
	return np.key, nil
}
