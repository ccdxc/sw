package services

import (
	"sync"

	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/utils/netutils"
)

// ipService provides ip configuration utilities on ethernet interfaces.
type ipService struct {
	sync.Mutex
}

// NewIPService provids a new IPService
func NewIPService() types.IPService {
	return &ipService{}
}

// HasIP returns true if the provided ip address is configured on any interface.
func (i *ipService) HasIP(ip string) (bool, error) {
	i.Lock()
	defer i.Unlock()
	return netutils.HasIP(ip)
}

// AddSecondaryIP adds the provided ip address as a secondary IP to an interface
// that can have it. If no interface can have it, it returns an error.
func (i *ipService) AddSecondaryIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	return netutils.AddSecondaryIP(ip)
}

// DeleteIP deletes the specified address from the interface it is found on.
func (i *ipService) DeleteIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	return netutils.DeleteIP(ip)
}
