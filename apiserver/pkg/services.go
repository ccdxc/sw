package apisrvpkg

import (
	"sync"

	"github.com/pensando/sw/apiserver"
)

// ServiceHdlr is a represantaion of a service for API server
type ServiceHdlr struct {
	sync.Mutex
	// Name is the name of the service
	Name string
	// Methods is a container for the Methods defined in the service.
	Methods map[string]apiserver.Method
	// enabled is set to true when the service is enabled.
	enabled bool
}

// Disable disables all futures requests on the service to be forbidden
func (s *ServiceHdlr) Disable() {
	s.Lock()
	defer s.Unlock()
	for _, m := range s.Methods {
		m.Disable()
	}
	s.enabled = false
}

// Enable sets the API invocations on the Service to be allowed.
func (s *ServiceHdlr) Enable() {
	s.Lock()
	defer s.Unlock()
	for _, m := range s.Methods {
		m.Enable()
	}
	s.enabled = true
}

// GetMethod retrieves a method in the service given the method name.
func (s *ServiceHdlr) GetMethod(n string) apiserver.Method {
	if v, ok := s.Methods[n]; ok {
		return v
	}
	return nil
}

// AddMethod is invoked by the generated code to populate the Service with Methods.
func (s *ServiceHdlr) AddMethod(n string, m apiserver.Method) apiserver.Method {
	s.Methods[n] = m
	return s.Methods[n]
}

// NewService initializes and returns a new service object.
func NewService(n string) apiserver.Service {
	return &ServiceHdlr{Name: n, Methods: make(map[string]apiserver.Method)}
}
