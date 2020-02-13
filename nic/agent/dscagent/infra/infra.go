// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package infra

import (
	"os"
	"path"
	"sync"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/log"
)

// API implents types.InfraAPI. This is a collection of all infra specific APIs that are common across pipelines
type API struct {
	sync.Mutex
	config                    types.DistributedServiceCardStatus
	primaryStore, backupStore emstore.Emstore
	ifUpdCh                   chan types.UpdateIfEvent
}

// NewInfraAPI returns a new instance of InfraAPI. First db path is interpreted as primary and the second is secondary
func NewInfraAPI(primaryDBPath, backupDBPath string) (*API, error) {
	var i API
	var err error
	if len(primaryDBPath) == 0 || len(backupDBPath) == 0 {
		log.Error(errors.Wrap(types.ErrMissingStorePaths, "Infra API: "))
		return nil, errors.Wrap(types.ErrMissingStorePaths, "Infra API: ")
	}

	if _, err := os.Stat(primaryDBPath); os.IsNotExist(err) {
		if err := os.MkdirAll(path.Dir(primaryDBPath), 600); err != nil {
			log.Error(errors.Wrapf(types.ErrDBPathCreate, "Infra API: Path: %s | Err: %v", primaryDBPath, err))
			return nil, errors.Wrapf(types.ErrDBPathCreate, "Infra API: Path: %s | Err: %v", primaryDBPath, err)
		}
	}

	if _, err := os.Stat(backupDBPath); os.IsNotExist(err) {
		if err := os.MkdirAll(path.Dir(backupDBPath), 600); err != nil {
			log.Error(errors.Wrapf(types.ErrDBPathCreate, "Infra API: Path: %s | Err: %v", backupDBPath, err))
			return nil, errors.Wrapf(types.ErrDBPathCreate, "Infra API: Path: %s | Err: %v", backupDBPath, err)
		}
	}

	i.primaryStore, err = emstore.NewEmstore(emstore.BoltDBType, primaryDBPath)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrPrimaryStoreCreate, "Infra API: Err: %v", err))
		return nil, errors.Wrapf(types.ErrPrimaryStoreCreate, "Infra API: Err: %v", err)
	}

	i.backupStore, err = emstore.NewEmstore(emstore.BoltDBType, backupDBPath)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBackupStoreCreate, "Infra API: Err: %v", err))
		return nil, errors.Wrapf(types.ErrBackupStoreCreate, "Infra API: Err: %v", err)
	}

	i.ifUpdCh = make(chan types.UpdateIfEvent, 100)
	return &i, nil
}

// List returns a list of objects by kind
func (i *API) List(kind string) ([][]byte, error) {
	return i.primaryStore.RawList(kind)
}

// Read returns a specific object based on key and kind
func (i *API) Read(kind, key string) ([]byte, error) {
	return i.primaryStore.RawRead(kind, key)
}

// Store stores a specific object based on key and kind
func (i *API) Store(kind, key string, data []byte) error {
	go func() {
		if i.backupStore != nil {
			if err := i.backupStore.RawWrite(kind, key, data); err != nil {
				log.Error(errors.Wrapf(types.ErrBackupStoreWrite, "Infra API: Err: %v", err))
			}
		}
	}()

	return i.primaryStore.RawWrite(kind, key, data)
}

// Delete deletes a specific object based on key and kind
func (i *API) Delete(kind, key string) error {
	go func() {
		if i.backupStore != nil {
			if err := i.backupStore.RawDelete(kind, key); err != nil {
				log.Error(errors.Wrapf(types.ErrBackupStoreDelete, "Infra API: Err: %v", err))
			}
		}
	}()

	return i.primaryStore.RawDelete(kind, key)
}

// AllocateID allocates a uint64 ID based on an offset.
func (i *API) AllocateID(kind emstore.ResourceType, offset int) uint64 {
	id, err := i.primaryStore.GetNextID(kind, offset)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrPrimaryStoreIDAlloc, "Infra API: Err: %v", err))
	}

	go func() {
		if i.backupStore != nil {
			_, err := i.backupStore.GetNextID(kind, offset)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrBackupStoreIDAlloc, "Infra API: Err: %v", err))
			}
		}
	}()
	return id
}

// GetDscName returns the DSC Name. Cluster-wide uniqueness is ensured by using the primary mac address from FRU
func (i *API) GetDscName() string {
	i.Lock()
	defer i.Unlock()
	return i.config.DSCName
}

// StoreConfig stores NetAgent config that it got from NMD
func (i *API) StoreConfig(config types.DistributedServiceCardStatus) {
	i.Lock()
	defer i.Unlock()
	i.config = config
}

// GetConfig stores NetAgent config that it got from NMD
func (i *API) GetConfig() types.DistributedServiceCardStatus {
	i.Lock()
	defer i.Unlock()
	return i.config
}

// Close releases locks on primary and back up boltDB
func (i *API) Close() error {
	if i.primaryStore != nil {
		if err := i.primaryStore.Close(); err != nil {
			log.Error(errors.Wrapf(types.ErrPrimaryStoreClose, "Infra API: %s", err))
			return errors.Wrapf(types.ErrPrimaryStoreClose, "Infra API: %s", err)
		}
	}

	if i.backupStore != nil {
		if err := i.backupStore.Close(); err != nil {
			log.Error(errors.Wrapf(types.ErrBackupStoreClose, "Infra API: %s", err))
			return errors.Wrapf(types.ErrBackupStoreClose, "Infra API: %s", err)
		}
	}

	return nil
}

// NotifyVeniceConnection marks venice connection status to true.
func (i *API) NotifyVeniceConnection() {
	i.Lock()
	defer i.Unlock()
	i.config.IsConnectedToVenice = true
}

// UpdateIfChannel returns a channel for propogating interface state to the netagent
func (i *API) UpdateIfChannel() chan types.UpdateIfEvent {
	return i.ifUpdCh
}
