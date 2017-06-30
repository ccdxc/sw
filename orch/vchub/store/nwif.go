package store

import (
	"context"
	swapi "github.com/pensando/sw/api"
	"github.com/pensando/sw/orch/vchub/api"
	"github.com/pensando/sw/utils/kvstore"
)

const (
	nwifPath = "/vchub/nwifs/"
)

// NwIFCreate creates a NwIF object in the kv store
func NwIFCreate(ctx context.Context, ID string, n *api.NwIF) error {
	key := nwifPath + ID
	return kvStore.Create(ctx, key, n)
}

// NwIFDelete deletes a NwIF object from the kv store
func NwIFDelete(ctx context.Context, ID string) error {
	key := nwifPath + ID
	return kvStore.Delete(ctx, key, nil)
}

// NwIFUpdate updates a NwIF object in the kv store
func NwIFUpdate(ctx context.Context, ID string, n *api.NwIF) error {
	key := nwifPath + ID
	return kvStore.Update(ctx, key, n)
}

// NwIFWatchAll is used for watching all NwIF objects in the kv store
func NwIFWatchAll(ctx context.Context, refVersion string) (kvstore.Watcher, error) {
	return kvStore.PrefixWatch(ctx, nwifPath, refVersion)
}

// NwIFList lists all NwIF objects in the kv store
func NwIFList(ctx context.Context) (*api.NwIFList, error) {
	list := api.NwIFList{ListMeta: &swapi.ListMeta{}}
	err := kvStore.List(ctx, nwifPath, &list)
	return &list, err
}
