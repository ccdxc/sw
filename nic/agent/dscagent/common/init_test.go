package common

import (
	"errors"
	"io/ioutil"
	"os"
	"testing"

	"github.com/pensando/sw/venice/utils/emstore"

	"github.com/pensando/sw/nic/agent/dscagent/infra"

	"github.com/pensando/sw/nic/agent/dscagent/types"

	"github.com/pensando/sw/venice/utils/log"
)

var (
	infraAPI types.InfraAPI
)

// Implements InfraAPI that return errors for testing
type badInfraAPI struct{}

// Sets up the grpc client handlers for the package
func TestMain(m *testing.M) {
	primaryDB, err := ioutil.TempFile("", "")
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	secondaryDB, err := ioutil.TempFile("", "")
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	infraAPI, err = infra.NewInfraAPI(primaryDB.Name(), secondaryDB.Name())
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	code := m.Run()
	infraAPI.Close()
	os.Remove(primaryDB.Name())
	os.Remove(secondaryDB.Name())
	os.Exit(code)
}

// newBadInfraAPI returns a new instance of InfraAPI
func newBadInfraAPI() *badInfraAPI {
	i := badInfraAPI{}
	return &i
}

// List returns a list of objects by kind
func (i *badInfraAPI) List(kind string) ([][]byte, error) {
	return nil, errors.New("failed to list")
}

// Read returns a specific object based on key and kind
func (i *badInfraAPI) Read(kind, key string) ([]byte, error) {
	return nil, errors.New("failed to read")
}

// Store stores a specific object based on key and kind
func (i *badInfraAPI) Store(kind, key string, data []byte) error {
	return errors.New("failed to store")
}

// Delete deletes a specific object based on key and kind
func (i *badInfraAPI) Delete(kind, key string) error {
	return errors.New("failed to delete")
}

// AllocateID allocates a uint64 ID based on an offset.
func (i *badInfraAPI) AllocateID(kind emstore.ResourceType, offset int) uint64 {
	return 0
}

// getDscName returns DSC Name
func (i *badInfraAPI) GetDscName() string {
	return "dsc"
}

// StoreConfig updates config
func (i *badInfraAPI) StoreConfig(cfg types.DistributedServiceCardStatus) {

}

// NotifyVeniceConnection notifies venice connection
func (i *badInfraAPI) NotifyVeniceConnection() {

}

// Purge stubbed out
func (i *badInfraAPI) Purge() {

}

func (i *badInfraAPI) GetConfig() (cfg types.DistributedServiceCardStatus) {
	return types.DistributedServiceCardStatus{
		DSCMode:     "",
		DSCName:     "",
		MgmtIP:      "",
		Controllers: nil,
	}
}

// UpdateIfChannel returns a channel for propogating interface state to the netagent
func (i *badInfraAPI) UpdateIfChannel() chan types.UpdateIfEvent {
	return nil
}

func (i *badInfraAPI) Close() error {
	return errors.New("failed to close")
}
