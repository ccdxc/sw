# kvstore

kvstore provides a key value(KV) store abstraction for working with various KV stores. The initial implementation only covers etcd. The focus is to provide Object semantics for common KV operations like Create, Delete, Update, Get, List and Watch operations.


## Creating a new kvstore client
```
import (
        "flag"
        "strings"

        log "github.com/Sirupsen/logrus"

        "github.com/pensando/sw/utils/kvstore/store"
        "github.com/pensando/sw/utils/runtime"
)

func main() {
	var etcdServers string
	flag.StringVar(&etcdServers, "etcd-servers", "http://localhost:2379", "comma seperated URLs for etcd servers")
	flag.Parse()

	s := runtime.NewScheme()
	s.AddKnownTypes(&User{}, &UserList{})

	config := store.Config{
		Type:    store.KVStoreTypeEtcd,
		Servers: strings.Split(etcdServers, ","),
		Codec:   runtime.NewJSONCodec(s),
	}

	kvStore, err := store.New(config)
	if err != nil {
		log.Fatalf("Failed to create store with error: %v", err)
	}
...
}
```

## Using the kvstore client
kvStore in the above snippet implements Interface which supports the following methods to work with the KV store. The interface is documented in interfaces.go

A full example is in examples/server/users.go
