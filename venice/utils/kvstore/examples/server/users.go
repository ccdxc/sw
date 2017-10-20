package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"net/http"
	"path"
	"strings"

	"github.com/go-martini/martini"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/store"
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

var (
	kvStore kvstore.Interface
)

const (
	usersURL = "/users"
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

	kv, err := store.New(config)
	if err != nil {
		log.Fatalf("Failed to create store with error: %v", err)
	}

	kvStore = kv

	mux := NewHTTPServer()
	port := ":9001"

	log.Infof("Starting http server at %v", port)
	mux.RunOnAddr(port)
}

// NewHTTPServer creates a http server for user API endpoints.
func NewHTTPServer() *martini.ClassicMartini {
	m := martini.Classic()

	m.Post(usersURL, UserCreateHandler)
	m.Delete(usersURL+"/:name", UserDeleteHandler)
	m.Get(usersURL+"/:name", UserGetHandler)
	m.Get(usersURL, UserListHandler)
	m.Get("/watch"+usersURL, UsersWatchHandler)

	return m
}

// UserCreateHandler creates a user.
func UserCreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	user := User{}
	if err := decoder.Decode(&user); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	key := path.Join(usersURL, user.Name)

	if err := kvStore.Create(context.Background(), key, &user); err != nil {
		if kvstore.IsKeyExistsError(err) {
			return http.StatusConflict, fmt.Sprintf("User %q already exists\n", user.Name)
		}
		return http.StatusBadRequest, err.Error()
	}

	return http.StatusOK, fmt.Sprintf("User %q created\n", user.Name)
}

// UserDeleteHandler deletes a user.
func UserDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	name := params["name"]

	key := path.Join(usersURL, name)

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("User %q deletion failed: %v\n", name, err)
	}
	return http.StatusOK, fmt.Sprintf("User %q deleted\n", name)
}

// UserGetHandler looks up a user.
func UserGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	name := params["name"]

	key := path.Join(usersURL, name)
	user := User{}

	if err := kvStore.Get(context.Background(), key, &user); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("User %q not found\n", name)
		}
		return http.StatusInternalServerError, fmt.Sprintf("User %q get failed with error: %v\n", name, err)
	}

	out, err := json.Marshal(&user)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// UserListHandler lists all users.
func UserListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	users := UserList{}

	if err := kvStore.List(context.Background(), usersURL, &users); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Users not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("Users list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&users)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// UsersWatchHandler establishes a watch on users hierarchy.
func UsersWatchHandler(w http.ResponseWriter, req *http.Request) {
	notifier, ok := w.(http.CloseNotifier)
	if !ok {
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		return
	}
	encoder := json.NewEncoder(w)
	watcher, err := kvStore.PrefixWatch(context.Background(), usersURL, "0")
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

			if err := encoder.Encode(event); err != nil {
				return
			}
			if len(ch) == 0 {
				flusher.Flush()
			}
		case <-notifier.CloseNotify():
			return
		}
	}
}
