package store

import (
	"strings"

	"github.com/pensando/sw/orch/vchub/api"
	"github.com/pensando/sw/utils/kvstore"
	ks "github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/sw/utils/runtime"
)

var kvStore kvstore.Interface

// Init initializes the VCHub store
func Init(servers, storeType string) (kvstore.Interface, error) {
	var err error
	s := runtime.NewScheme()
	s.AddKnownTypes(&api.SmartNIC{ObjectKind: "SmartNIC"}, &api.SmartNICList{})
	s.AddKnownTypes(&api.NwIF{ObjectKind: "NwIF"}, &api.NwIFList{})

	config := ks.Config{
		Type:    storeType,
		Servers: strings.Split(servers, ","),
		Codec:   runtime.NewJSONCodec(s),
	}

	kvStore, err = ks.New(config)
	return kvStore, err
}
