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

func NwIF_Create(ctx context.Context, ID string, n *api.NwIF) error {
	key := nwifPath + ID
	return kvStore.Create(ctx, key, n)
}

func NwIF_Delete(ctx context.Context, ID string) error {
	key := nwifPath + ID
	return kvStore.Delete(ctx, key, nil)
}

func NwIF_Update(ctx context.Context, ID string, n *api.NwIF) error {
	key := nwifPath + ID
	return kvStore.Update(ctx, key, n)
}

func NwIF_WatchAll(ctx context.Context, refVersion string) (kvstore.Watcher, error) {
	return kvStore.PrefixWatch(ctx, nwifPath, refVersion)
}

func NwIF_List(ctx context.Context) (*api.NwIFList, error) {
	list := api.NwIFList{ListMeta: &swapi.ListMeta{}}
	err := kvStore.List(ctx, nwifPath, &list)
	return &list, err
}
