package services

import (
	"fmt"
	"sync"

	"github.com/pensando/sw/cmd/types"
)

// mockIPService is used by tests for faking interface configuration.
type mockIPService struct {
	sync.Mutex
	ip    string
	error bool
}

// NewMockIPService returns a Mock IPService
func NewMockIPService() types.IPService {
	return &mockIPService{}
}

//HasIP returns true if this IP was added before
func (i *mockIPService) HasIP(ip string) (bool, error) {
	i.Lock()
	defer i.Unlock()
	if i.error {
		return false, fmt.Errorf("Error set")
	}
	return i.ip == ip, nil
}

// AddSecondaryIP mocks addition of IP address
func (i *mockIPService) AddSecondaryIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	if i.error {
		return fmt.Errorf("Error set")
	}
	if i.ip != "" {
		return fmt.Errorf("IP already configured")
	}
	i.ip = ip
	return nil
}

// DeleteIP deletes previously added IP
func (i *mockIPService) DeleteIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	if i.error {
		return fmt.Errorf("Error set")
	}
	i.ip = ""
	return nil
}

// SetError sets an error
func (i *mockIPService) SetError() {
	i.Lock()
	defer i.Unlock()
	i.error = true
}

// ClearError clears an error
func (i *mockIPService) ClearError() {
	i.Lock()
	defer i.Unlock()
	i.error = false
}
