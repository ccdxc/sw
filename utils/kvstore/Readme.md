# kvstore

kvstore provides a key value(KV) store abstraction for working with various KV stores. The initial implementation only covers etcd. The focus is to provide Object semantics for common KV operations like Create, Delete, Update, Get, List and Watch operations.


## Creating a new kvstore client
```go
import (
        "flag"
        "strings"

        log "github.com/Sirupsen/logrus"

        "github.com/pensando/sw/utils/kvstore/store"
        "github.com/pensando/sw/utils/runtime"
)

// User represents a user.
type User struct {
	api.TypeMeta   `json:",inline"`
	api.ObjectMeta `json:"meta"`
}

// UserList is a list of users.
type UserList struct {
	api.TypeMeta `json:",inline"`
	api.ListMeta `json:"meta"`
	Items        []User
}

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

### Create an object
```go
	if err := kvStore.Create(key, &user, 0, &user); err != nil {
		if kvstore.IsKeyExistsError(err) {
		   ...
		}
		...
	}
```

### Get an object
```go
	if err := kvStore.Get(key, &user); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
		   ...
		}
		...
	}
```

### List all objects under a hierarchy
```go
	users := UserList{}

	if err := kvStore.List(usersURL, &users); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
		   ...
		}
		...
	}
```

### Delete an object
```go
	if err := kvStore.Delete(key, nil); err != nil {
	   ...
	}
```

### Watch a hierarchy
```go
	watcher, err := kvStore.PrefixWatch(usersURL, "0")
	if err != nil {
		return
	}
	ch := watcher.EventChan()
	for {
		select {
		case event, ok := <-ch:
			if !ok {
				return
			}
			...
		}
	}
```

### Contest in an election
```go
	election, err := kvStore.Contest(electionName, id, ttl)
	if err != nil {
		log.Fatalf("Failed to start the leader election with error: %v", err)
	}

	for {
		select {
		case e := <-election.EventChan():
			log.Infof("Election event: %+v", e)
		}
	}
```

Working examples are in the examples directory.
