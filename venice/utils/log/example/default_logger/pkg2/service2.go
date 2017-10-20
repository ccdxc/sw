package pkg2

import (
	"os"

	"github.com/pensando/sw/venice/utils/log"
)

// Run starts the service
func Run() error {
	log.Info("started service2")
	log.Debug("svc2 data foo")
	return nil
}

// Close terminates the service
func Close() error {
	log.Errorf("failed to terminate service2: %v", os.ErrInvalid)
	return os.ErrInvalid
}
