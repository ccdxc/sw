package services

import (
	"sync"

	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/utils/net"
)

// ipService provides ip configuration utilities on ethernet interfaces.
type ipService struct {
	sync.Mutex
}

func NewIPService() types.IPService {
	return &ipService{}
}

func (i *ipService) HasIP(ip string) (bool, error) {
	i.Lock()
	defer i.Unlock()
	return net.HasIP(ip)
}

func (i *ipService) AddSecondaryIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	return net.AddSecondaryIP(ip)
}

func (i *ipService) DeleteIP(ip string) error {
	i.Lock()
	defer i.Unlock()
	return net.DeleteIP(ip)
}
