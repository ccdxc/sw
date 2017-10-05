package services

import (
	"sync"

	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/log"
)

// vipService provides ip configuration utilities on ethernet interfaces.
type vipService struct {
	sync.Mutex
	leaderSvc  types.LeaderService
	virtualIPs map[string]interface{}
	isLeader   bool // implementation will eventually use multiple leaders
	ipservice  types.IPService
}

// VIPOption fills the optional params
type VIPOption func(service *vipService)

// WithIPServiceVIPOption to pass a specifc types.IPService implementation
func WithIPServiceVIPOption(ipSvc types.IPService) VIPOption {
	return func(o *vipService) {
		o.ipservice = ipSvc
	}
}

// WithLeaderServiceVIPOption to pass a specifc types.LeaderService implementation
func WithLeaderServiceVIPOption(leaderSvc types.LeaderService) VIPOption {
	return func(o *vipService) {
		o.leaderSvc = leaderSvc
	}
}

// NewVIPService provids a new vipService
func NewVIPService(options ...VIPOption) types.VIPService {
	vip := vipService{
		virtualIPs: make(map[string]interface{}),
		ipservice:  NewIPService(),
		leaderSvc:  env.LeaderService,
	}

	for _, o := range options {
		if o != nil {
			o(&vip)
		}
	}
	if vip.leaderSvc == nil {
		panic("Current implementation of VIP Service needs a global Leaderservice")
	}

	vip.leaderSvc.Register(&vip)
	return &vip
}

// GetAllVirtualIPs returns the list of All VirtualIP that this node can potentially configure if there
// are no other nodes in the network (and hence this node is the leader)
func (i *vipService) GetAllVirtualIPs() []string {
	i.Lock()
	defer i.Unlock()
	var retval []string
	for k := range i.virtualIPs {
		retval = append(retval, k)
	}
	return retval
}

// GetConfiguredVirtualIPs returns the VirtualIPs that are configured on this node. Some VirtualIPs may
// be configured on other nodes on the cluster
func (i *vipService) GetConfiguredVirtualIPs() []string {
	i.Lock()
	defer i.Unlock()
	var retval []string
	for k := range i.virtualIPs {
		if present, err := i.ipservice.HasIP(k); err == nil && present {
			retval = append(retval, k)
		}
	}
	return retval
}

// AddVirtualIPs adds a set of virtualIPs to the cluster VIP service. Some of the VIP may be configured
// on this node and some on other nodes in the cluster.
func (i *vipService) AddVirtualIPs(vips ...string) error {
	i.Lock()
	defer i.Unlock()
	var err error
	for _, ip := range vips {
		i.virtualIPs[ip] = nil
		// TODO: When VIP service runs its own leaderService, start a per-VIP based leaderService
		if i.isLeader {
			e := i.ipservice.AddSecondaryIP(ip)
			if e != nil && err == nil {
				err = e
			}
		}
	}
	return err
}

// DeleteVirtualIPs removes the set of virtualIPs from the cluster VIP service. Some of the VIP may be configured
// on this node and some on other nodes in the cluster
func (i *vipService) DeleteVirtualIPs(vips ...string) error {
	i.Lock()
	defer i.Unlock()
	var err error
	for _, ip := range vips {
		delete(i.virtualIPs, ip)
		if i.isLeader {
			er := i.ipservice.DeleteIP(ip)
			if er != nil && err == nil {
				err = er
			}
		}
	}
	return nil
}

func (i *vipService) OnNotifyLeaderEvent(e types.LeaderEvent) error {
	i.Lock()
	defer i.Unlock()

	var err, er error
	switch e.Evt {
	case types.LeaderEventWon:
		i.isLeader = true
		for ip := range i.virtualIPs {
			var found bool
			if found, er = i.ipservice.HasIP(ip); er != nil {
				log.Errorf("unable to determine if virtual ip is already present. error: %v", er)
			} else if !found {
				if er = i.ipservice.AddSecondaryIP(ip); er != nil {
					log.Errorf("leader node unable to Add virtual IP. error: %v", er)
				}
			}
			if er != nil && err == nil {
				err = er
			}
		}
		// TODO: In a go routine, send GratARP few times to take care of any lost GratARP - making clients
		// converge faster.

		// On CMD restart(in case of a crash), vip service comes back up and participates in
		// election. If some other node becomes leader in this case, LeaderEventChange is sent
		// However we might have been leader before crash. So cleanup any virtualIP.
	case types.LeaderEventChange:
		fallthrough
	case types.LeaderEventLost:
		i.isLeader = false
		for ip := range i.virtualIPs {
			if er = i.ipservice.DeleteIP(ip); er != nil {
				log.Errorf("Failed to delete virtual IP with error: %v", er)
			}
		}
		if er != nil && err == nil {
			err = er
		}
	}
	return err
}
