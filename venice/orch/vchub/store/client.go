package store

import (
	"context"
	"errors"
	"strings"
	"time"

	"github.com/pensando/sw/venice/orch"
	"github.com/pensando/sw/venice/utils/kvstore"
	ks "github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	retryDelay = 500 * time.Millisecond
	maxRetries = 5
)

var kvStore kvstore.Interface
var config ks.Config

// Init sets up a kvstore client
func Init(servers, storeType string) (kvstore.Interface, error) {
	var err error
	s := runtime.NewScheme()
	s.AddKnownTypes(&orch.SmartNIC{ObjectKind: "SmartNIC"}, &orch.SmartNICList{})
	s.AddKnownTypes(&orch.NwIF{ObjectKind: "NwIF"}, &orch.NwIFList{})

	config = ks.Config{
		Type:    storeType,
		Servers: strings.Split(servers, ","),
		Codec:   runtime.NewJSONCodec(s),
	}

	kvStore, err = ks.New(config)
	return kvStore, err
}

// Reinit is used to create a new client upon error
func Reinit() {
	time.Sleep(retryDelay)
	kvStore, _ = ks.New(config)
}

// Close closes the client session
func Close() {
	kvStore.Close()
}

// kvCreate is a wrapper that retries on transient kv errors
func kvCreate(ctx context.Context, key string, obj runtime.Object) error {
	log.Infof("kvCreate key: %s, obj: %+v", key, obj)
	for i := 0; i < maxRetries; i++ {
		err := kvStore.Create(ctx, key, obj)
		if err == nil || kvstore.IsKeyExistsError(err) || ctx.Err() != nil {
			return err
		}

		Reinit()
	}

	return errors.New("Exceeded max retries")
}

// kvUpdate is a wrapper that retries on transient kv errors
func kvUpdate(ctx context.Context, key string, obj runtime.Object) error {
	log.Infof("kvUpdate key: %s, obj: %+v", key, obj)
	for i := 0; i < maxRetries; i++ {
		err := kvStore.Update(ctx, key, obj)
		if err == nil || kvstore.IsKeyNotFoundError(err) || ctx.Err() != nil {
			return err
		}

		Reinit()
	}

	return errors.New("Exceeded max retries")
}

// kvGet is a wrapper that retries on transient kv errors
func kvGet(ctx context.Context, key string, obj runtime.Object) error {
	log.Infof("kvGet key: %s, obj: %+v", key, obj)
	for i := 0; i < maxRetries; i++ {
		err := kvStore.Get(ctx, key, obj)
		if err == nil || kvstore.IsKeyNotFoundError(err) || ctx.Err() != nil {
			return err
		}

		Reinit()
	}

	return errors.New("Exceeded max retries")
}

// kvDelete is a wrapper that retries on transient kv errors
func kvDelete(ctx context.Context, key string) error {
	log.Infof("kvDelete key: %s", key)
	for i := 0; i < maxRetries; i++ {
		err := kvStore.Delete(ctx, key, nil)
		if err == nil || kvstore.IsKeyNotFoundError(err) || ctx.Err() != nil {
			return err
		}

		Reinit()
	}

	return errors.New("Exceeded max retries")
}
