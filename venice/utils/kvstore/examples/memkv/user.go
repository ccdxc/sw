package main

import (
	"context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
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

const (
	usersURL = "/users"
)

func main() {
	s := runtime.NewScheme()
	s.AddKnownTypes(&User{}, &UserList{})

	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(s)}

	kv, err := store.New(config)
	if err != nil {
		log.Fatalf("Failed to create store with error: %v", err)
	}

	key := "/users/joe"
	user := User{}
	if err := kv.Create(context.Background(), key, &user); err != nil {
		log.Errorf("error storing key in the key-value store")
	}
	if err := kv.Get(context.Background(), key, &user); err != nil {
		log.Errorf("error getting key in the key-value store")
	}

	log.Printf("created and got a sample key: %s \n", key)
}
