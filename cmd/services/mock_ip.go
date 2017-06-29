package services

import (
	"sync"

	"github.com/pensando/sw/cmd/types"
)

// mockIPService is used by tests for faking interface configuration.
type mockIPService struct {
	sync.Mutex
	ip string
}

// NewMockIPService returns a Mock IPService
func NewMockIPService() types.IPService {
	return &mockIPService{}
}

//HasIP returns true if this IP was added before
func (i *mockIPService) HasIP(ip string) (bool, error) {
	i.Lock()
	defer i.Unlock()
	return i.ip == ip, nil
}

// AddSecondaryIP mocks addition of IP address
func (i *mockIPService) AddSecondaryIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	i.ip = ip
	return nil
}

// DeleteIP deletes previously added IP
func (i *mockIPService) DeleteIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	i.ip = ""
	return nil
}
