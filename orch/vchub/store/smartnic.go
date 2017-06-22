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

func SmartNIC_Create(ctx context.Context, ID string, s *api.SmartNIC) error {
	key := smartNICPath + ID
	return kvStore.Create(ctx, key, s)
}

func SmartNIC_Delete(ctx context.Context, ID string) error {
	key := smartNICPath + ID
	return kvStore.Delete(ctx, key, nil)
}

func SmartNIC_Update(ctx context.Context, ID string, s *api.SmartNIC) error {
	key := smartNICPath + ID
	return kvStore.Update(ctx, key, s)
}

func SmartNIC_WatchAll(ctx context.Context, refVersion string) (kvstore.Watcher, error) {
	return kvStore.PrefixWatch(ctx, smartNICPath, refVersion)
}

func SmartNIC_List(ctx context.Context) (*api.SmartNICList, error) {
	list := api.SmartNICList{ListMeta: &swapi.ListMeta{}}
	err := kvStore.List(ctx, smartNICPath, &list)
	return &list, err
}
