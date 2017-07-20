package store

import (
	"context"
	"reflect"

	log "github.com/Sirupsen/logrus"

	swapi "github.com/pensando/sw/api"
	"github.com/pensando/sw/orch/vchub/api"
	"github.com/pensando/sw/orch/vchub/defs"
	"github.com/pensando/sw/utils/kvstore"
)

const (
	smartNICPath = "/vchub/smartnics/"
	smartNICKind = "SmartNIC"
)

// SNICStore maintains information about a store instance
type SNICStore struct {
	ctx   context.Context
	hosts map[string]*defs.ESXHost
}

// SmartNICCreate creates a SmartNIC object in the kv store
func SmartNICCreate(ctx context.Context, ID string, s *api.SmartNIC) error {
	key := smartNICPath + ID

	//if the object already exists, compare and update only if there is a change.
	old := &api.SmartNIC{}
	err := kvStore.Get(ctx, key, old)

	if err == nil {
		s.ObjectMeta.ResourceVersion = old.ObjectMeta.ResourceVersion
		if reflect.DeepEqual(old, s) {
			return nil
		}

		log.Infof("vchub.store SmartNICCreate: old %+v new %+v", old, s)

		return kvUpdate(ctx, key, s)
	}

	log.Infof("vchub.store SmartNICCreate: %+v", s)
	return kvCreate(ctx, key, s)
}

// SmartNICDelete deletes a SmartNIC object from the kv store
func SmartNICDelete(ctx context.Context, ID string) error {
	log.Infof("vchub.store SmartNICDelete: %+v", ID)
	key := smartNICPath + ID
	return kvDelete(ctx, key)
}

// SmartNICUpdate updates a SmartNIC object in the kv store
func SmartNICUpdate(ctx context.Context, ID string, s *api.SmartNIC) error {
	log.Infof("vchub.store SmartNICUpdate: %+v %+v", ID, s)
	key := smartNICPath + ID
	return kvUpdate(ctx, key, s)
}

// SmartNICWatchAll is used for watching all SmartNIC objects in the kv store
func SmartNICWatchAll(ctx context.Context, refVersion string) (kvstore.Watcher, error) {
	return kvStore.PrefixWatch(ctx, smartNICPath, refVersion)
}

// SmartNICList lists all SmartNIC objects in the kv store
func SmartNICList(ctx context.Context) (*api.SmartNICList, error) {
	list := api.SmartNICList{ListMeta: &swapi.ListMeta{}}
	err := kvStore.List(ctx, smartNICPath, &list)
	return &list, err
}

// NewSNICStore returns an instance of SNICStore
func NewSNICStore() *SNICStore {
	return &SNICStore{hosts: make(map[string]*defs.ESXHost)}
}

// Run processes updates sent on the input channel
func (s *SNICStore) Run(ctx context.Context, inbox <-chan defs.HostMsg) {
	s.ctx = ctx

	for {
		select {
		case <-ctx.Done():
			return

		case m, active := <-inbox:
			if !active {
				return
			}

			log.Infof("Msg from %v, key: %s", m.Originator, m.Key)
			switch m.Op {
			case defs.VCOpDelete:
				s.deleteHost(m.Key)

			case defs.VCOpSet:
				s.setHost(m.Key, m.Value)
			}
		}
	}
}

// deleteHost performs necessary store updates when an esx host is removed.
func (s *SNICStore) deleteHost(hostKey string) {
	// find all nics of this host and delete them
	h := s.hosts[hostKey]
	if h == nil {
		return
	}

	for _, nic := range h.PenNICs {
		err := SmartNICDelete(s.ctx, nic.Mac)
		if err != nil {
			log.Errorf("SmartNICDelete returned %v", err)
		}
	}

	delete(s.hosts, hostKey)
}

// setHost performs necessary store updates when an esx host is added/changed.
func (s *SNICStore) setHost(hostKey string, host *defs.ESXHost) {
	currHost := s.hosts[hostKey]
	if reflect.DeepEqual(currHost, host) {
		return // no change
	}

	// create all current snics
	for _, nic := range host.PenNICs {
		snic := &api.SmartNIC{
			ObjectKind:       smartNICKind,
			ObjectAPIVersion: "v1",
			ObjectMeta:       &swapi.ObjectMeta{},
			Status: &api.SmartNIC_Status{
				HostIP:     host.HostIP,
				MacAddress: nic.Mac,
				Switch:     nic.DvsUUID,
			},
		}
		err := SmartNICCreate(s.ctx, nic.Mac, snic)
		if err != nil {
			log.Errorf("SmartNICCreate returned %v", err)
		}
	}

	// delete any old ones that are not present now
	if currHost != nil {
		for m, nic := range currHost.PenNICs {
			if _, found := host.PenNICs[m]; !found {
				err := SmartNICDelete(s.ctx, nic.Mac)
				if err != nil {
					log.Errorf("SmartNICDelete returned %v", err)
				}
			}
		}
	}

	s.hosts[hostKey] = host
}
