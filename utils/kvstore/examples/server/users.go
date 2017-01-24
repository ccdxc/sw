package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"net/http"
	"path"
	"strings"

	log "github.com/Sirupsen/logrus"
	"github.com/go-martini/martini"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/sw/utils/runtime"
)

type User struct {
	api.TypeMeta   `json:",inline"`
	api.ObjectMeta `json:"meta"`
}

type UserList struct {
	api.TypeMeta `json:",inline"`
	api.ListMeta `json:"meta"`
	Items        []User
}

var (
	kvStore kvstore.Interface
)

const (
	UsersURL = "/users"
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

	mux := NewHttpServer()
	port := ":9001"

	log.Infof("Starting http server at %v", port)
	mux.RunOnAddr(port)
}

// NewHttpServer creates a http server for user API endpoints.
func NewHttpServer() *martini.ClassicMartini {
	m := martini.Classic()

	m.Post(UsersURL, UserCreateHandler)
	m.Delete(UsersURL+"/:name", UserDeleteHandler)
	m.Get(UsersURL+"/:name", UserGetHandler)
	m.Get(UsersURL, UserListHandler)
	m.Get("/watch"+UsersURL, UsersWatchHandler)

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

	key := path.Join(UsersURL, user.Name)

	if err := kvStore.Create(key, &user, 0, &user); err != nil {
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

	key := path.Join(UsersURL, name)

	if err := kvStore.Delete(key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("User %q deletion failed: %v\n", name, err)
	}
	return http.StatusOK, fmt.Sprintf("User %q deleted\n", name)
}

// UserGetHandler looks up a user.
func UserGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	name := params["name"]

	key := path.Join(UsersURL, name)
	user := User{}

	if err := kvStore.Get(key, &user); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("User %q not found\n", name)
		}
		return http.StatusInternalServerError, fmt.Sprintf("User %q get failed with error: %v\n", name, err)
	}

	if out, err := json.Marshal(&user); err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	} else {
		return http.StatusOK, string(out)
	}
}

// UserListHandler lists all users.
func UserListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	users := UserList{}

	if err := kvStore.List(UsersURL, &users); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Users not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("Users list failed with error: %v\n", err)
	}

	if out, err := json.Marshal(&users); err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	} else {
		return http.StatusOK, string(out)
	}
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
	watcher, err := kvStore.PrefixWatch(UsersURL, "0")
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
