package idgen

import (
	"context"
	"strings"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/idgen/pb"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/runtime"
)

// IDType is the type of ID generation
type IDType int

// ID Types
const (
	SEQUENTIAL IDType = 0
)

func (t *IDType) String() string {
	switch *t {
	case SEQUENTIAL:
		return "sequential"
	default:
		return "unknown"
	}
}

func (t *IDType) prefix() string {
	switch *t {
	case SEQUENTIAL:
		return "seq"
	default:
		return "unknown"
	}
}

// Config setups up the global id generator configuration
type Config struct {
	kvconfig store.Config
}

var config *Config

var (
	errInvalidParameters = errors.New("invalid parameters")
)

// Generator defines the interface for id generation
type Generator interface {
	New() (uint64, error)
	Close()
}

type idGen struct {
	ctx    context.Context
	cancel context.CancelFunc
	kvs    kvstore.Interface
	key    string
	idtype int
	lastid uint64
}

func (i *idGen) New() (uint64, error) {
	genobj := idgenpb.IDGenObj{}
	updateFunc := func(oldObj runtime.Object) (newObj runtime.Object, err error) {
		o := oldObj.(*idgenpb.IDGenObj)
		o.Info.LastId = o.Info.LastId + 1
		return o, nil
	}
	err := i.kvs.ConsistentUpdate(i.ctx, i.key, &genobj, updateFunc)
	if err != nil {
		return 0, err
	}
	return genobj.Info.LastId, nil
}

func (i *idGen) Close() {
	i.cancel()
	i.kvs.Close()
}

// New creates a new generator
func New(ctx context.Context, name string, tpe IDType) (Generator, error) {
	if name == "" || tpe < SEQUENTIAL || tpe > SEQUENTIAL {
		return nil, errInvalidParameters
	}

	key := globals.IDGeneratorsPath + "/" + tpe.prefix() + "/" + strings.TrimPrefix(name, "/")

	idgen := idGen{
		key:    key,
		idtype: int(tpe),
	}

	kvs, err := store.New(config.kvconfig)
	if err != nil {
		return nil, err
	}
	idgen.kvs = kvs
	// Fetch ID if it exists in kvstore, if not create a key
	kvobj := idgenpb.IDGenObj{}
	err = kvs.Get(ctx, key, &kvobj)
	if err != nil {
		// Object not found. Try and create a new ID Generator
		kvobj.Kind = "IDGenObj"
		kvobj.Name = name
		kvobj.Info.Type = int32(tpe)
		kvobj.Info.LastId = 0
		err = kvs.Create(ctx, key, &kvobj)
		if err != nil {
			return nil, err
		}
		idgen.ctx, idgen.cancel = context.WithCancel(ctx)
	} else {
		idgen.ctx, idgen.cancel = context.WithCancel(ctx)
		idgen.lastid = kvobj.Info.LastId
	}
	return &idgen, nil
}

// Destroy destroys an existing generator
func Destroy(ctx context.Context, name string, tpe IDType) error {
	if name == "" || tpe < SEQUENTIAL || tpe > SEQUENTIAL {
		return errInvalidParameters
	}

	key := globals.IDGeneratorsPath + "/" + tpe.prefix() + "/" + strings.TrimPrefix(name, "/")
	kvs, err := store.New(config.kvconfig)
	if err != nil {
		return err
	}
	err = kvs.Delete(ctx, key, nil)
	return err
}

// Init initializes the generator module.
func Init(cfg Config) {
	rcfg := cfg
	config = &rcfg
}
