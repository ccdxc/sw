package pkg2

import (
	"os"

	log "github.com/pensando/sw/venice/utils/log"
)

// Instance2 definition
type Instance2 struct {
	logger log.Logger
}

// NewInstance creates new pkg2 instance
func NewInstance(logObj log.Logger) *Instance2 {

	// Add more granular package specific context
	logger := logObj
	logger = logger.WithContext("pkg", "pkg2")

	obj := &Instance2{
		logger,
	}
	return obj
}

// Run starts the service
func (s *Instance2) Run() error {
	s.logger.Info("started service")
	s.logger.Debug("data foo")
	return nil
}

// Close terminates the service
func (s *Instance2) Close() error {
	s.logger.Errorf("failed to terminate service: %v", os.ErrInvalid)
	return os.ErrInvalid
}
