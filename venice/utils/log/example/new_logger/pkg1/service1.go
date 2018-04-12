package pkg1

import (
	log "github.com/pensando/sw/venice/utils/log"
)

// Instance1 definition
type Instance1 struct {
	logger log.Logger
}

// NewInstance creates new pkg2 instance
func NewInstance(logObj log.Logger) *Instance1 {

	// Add more granular package specific context
	logger := logObj
	logger = logger.WithContext("pkg", "pkg1", "svc", "svc1")

	obj := &Instance1{
		logger,
	}
	return obj
}

// Run starts the service
func (s *Instance1) Run() error {
	s.logger.Info("started service")
	s.logger.Debug("data bar")
	return nil
}

// Close terminates the service
func (s *Instance1) Close() error {
	s.logger.Info("terminated service")
	return nil
}
