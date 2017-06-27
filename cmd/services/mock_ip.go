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

func NewMockIPService() types.IPService {
	return &mockIPService{}
}

func (i *mockIPService) HasIP(ip string) (bool, error) {
	i.Lock()
	defer i.Unlock()
	return i.ip == ip, nil
}

func (i *mockIPService) AddSecondaryIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	i.ip = ip
	return nil
}

func (i *mockIPService) DeleteIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	i.ip = ""
	return nil
}
