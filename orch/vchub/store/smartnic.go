package store

import (
	"context"
	swapi "github.com/pensando/sw/api"
	"github.com/pensando/sw/orch/vchub/api"
	"github.com/pensando/sw/utils/kvstore"
)

const (
	smartNICPath = "/vchub/smartnics/"
)

// SmartNICCreate creates a SmartNIC object in the kv store
func SmartNICCreate(ctx context.Context, ID string, s *api.SmartNIC) error {
	key := smartNICPath + ID
	return kvStore.Create(ctx, key, s)
}

// SmartNICDelete deletes a SmartNIC object from the kv store
func SmartNICDelete(ctx context.Context, ID string) error {
	key := smartNICPath + ID
	return kvStore.Delete(ctx, key, nil)
}

// SmartNICUpdate updates a SmartNIC object in the kv store
func SmartNICUpdate(ctx context.Context, ID string, s *api.SmartNIC) error {
	key := smartNICPath + ID
	return kvStore.Update(ctx, key, s)
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
