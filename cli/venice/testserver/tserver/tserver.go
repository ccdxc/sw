// (c) Pensando Systems, Inc.
// This is a generated file, please do not hand edit !!

package tserver

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"path"
	"strings"

	log "github.com/Sirupsen/logrus"
	"github.com/go-martini/martini"
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/cli/api"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/sw/utils/runtime"
	"github.com/satori/go.uuid"
)

var (
	kvStore kvstore.Interface
)

// Start spins up a test server on the local host on the specified port
func Start(port string) {
	s := runtime.NewScheme()

	s.AddKnownTypes(&cmd.Cluster{}, &cmd.ClusterList{})

	s.AddKnownTypes(&network.Endpoint{}, &network.EndpointList{})

	s.AddKnownTypes(&network.LbPolicy{}, &network.LbPolicyList{})

	s.AddKnownTypes(&network.Network{}, &network.NetworkList{})

	s.AddKnownTypes(&cmd.Node{}, &cmd.NodeList{})

	s.AddKnownTypes(&api.Permission{}, &api.PermissionList{})

	s.AddKnownTypes(&api.Role{}, &api.RoleList{})

	s.AddKnownTypes(&network.SecurityGroup{}, &network.SecurityGroupList{})

	s.AddKnownTypes(&network.Service{}, &network.ServiceList{})

	s.AddKnownTypes(&network.Sgpolicy{}, &network.SgpolicyList{})

	s.AddKnownTypes(&network.Tenant{}, &network.TenantList{})

	s.AddKnownTypes(&api.User{}, &api.UserList{})

	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(s)}

	kv, err := store.New(config)
	if err != nil {
		log.Fatalf("Failed to create store with error: %v", err)
	}

	kvStore = kv

	mux := NewHTTPServer()

	log.Infof("Starting http server at %v", port)
	go mux.RunOnAddr(port)
}

// NewHTTPServer creates a http server for API endpoints.
func NewHTTPServer() *martini.ClassicMartini {
	m := martini.Classic()

	m.Post("/test"+api.Objs["cluster"].URL, ClusterCreateHandler)
	m.Post(api.Objs["cluster"].URL, ClusterCreateHandler)
	m.Put(api.Objs["cluster"].URL+"/:name", ClusterCreateHandler)
	m.Delete("/test"+api.Objs["cluster"].URL+"/:name", ClusterTestDeleteHandler)
	m.Delete(api.Objs["cluster"].URL+"/:name", ClusterActualDeleteHandler)
	m.Get(api.Objs["cluster"].URL+"/:name", ClusterGetHandler)
	m.Get(api.Objs["cluster"].URL, ClusterListHandler)
	m.Get("/watch"+api.Objs["cluster"].URL, ClustersWatchHandler)

	m.Post("/test"+api.Objs["endpoint"].URL, EndpointCreateHandler)
	m.Post(api.Objs["endpoint"].URL, EndpointCreateHandler)
	m.Put(api.Objs["endpoint"].URL+"/:name", EndpointCreateHandler)
	m.Delete("/test"+api.Objs["endpoint"].URL+"/:name", EndpointTestDeleteHandler)
	m.Delete(api.Objs["endpoint"].URL+"/:name", EndpointActualDeleteHandler)
	m.Get(api.Objs["endpoint"].URL+"/:name", EndpointGetHandler)
	m.Get(api.Objs["endpoint"].URL, EndpointListHandler)
	m.Get("/watch"+api.Objs["endpoint"].URL, EndpointsWatchHandler)

	m.Post("/test"+api.Objs["lbPolicy"].URL, LbPolicyCreateHandler)
	m.Post(api.Objs["lbPolicy"].URL, LbPolicyCreateHandler)
	m.Put(api.Objs["lbPolicy"].URL+"/:name", LbPolicyCreateHandler)
	m.Delete("/test"+api.Objs["lbPolicy"].URL+"/:name", LbPolicyTestDeleteHandler)
	m.Delete(api.Objs["lbPolicy"].URL+"/:name", LbPolicyActualDeleteHandler)
	m.Get(api.Objs["lbPolicy"].URL+"/:name", LbPolicyGetHandler)
	m.Get(api.Objs["lbPolicy"].URL, LbPolicyListHandler)
	m.Get("/watch"+api.Objs["lbPolicy"].URL, LbPolicysWatchHandler)

	m.Post("/test"+api.Objs["network"].URL, NetworkCreateHandler)
	m.Post(api.Objs["network"].URL, NetworkCreateHandler)
	m.Put(api.Objs["network"].URL+"/:name", NetworkCreateHandler)
	m.Delete("/test"+api.Objs["network"].URL+"/:name", NetworkTestDeleteHandler)
	m.Delete(api.Objs["network"].URL+"/:name", NetworkActualDeleteHandler)
	m.Get(api.Objs["network"].URL+"/:name", NetworkGetHandler)
	m.Get(api.Objs["network"].URL, NetworkListHandler)
	m.Get("/watch"+api.Objs["network"].URL, NetworksWatchHandler)

	m.Post("/test"+api.Objs["node"].URL, NodeCreateHandler)
	m.Post(api.Objs["node"].URL, NodeCreateHandler)
	m.Put(api.Objs["node"].URL+"/:name", NodeCreateHandler)
	m.Delete("/test"+api.Objs["node"].URL+"/:name", NodeTestDeleteHandler)
	m.Delete(api.Objs["node"].URL+"/:name", NodeActualDeleteHandler)
	m.Get(api.Objs["node"].URL+"/:name", NodeGetHandler)
	m.Get(api.Objs["node"].URL, NodeListHandler)
	m.Get("/watch"+api.Objs["node"].URL, NodesWatchHandler)

	m.Post("/test"+api.Objs["permission"].URL, PermissionCreateHandler)
	m.Post(api.Objs["permission"].URL, PermissionCreateHandler)
	m.Put(api.Objs["permission"].URL+"/:name", PermissionCreateHandler)
	m.Delete("/test"+api.Objs["permission"].URL+"/:name", PermissionTestDeleteHandler)
	m.Delete(api.Objs["permission"].URL+"/:name", PermissionActualDeleteHandler)
	m.Get(api.Objs["permission"].URL+"/:name", PermissionGetHandler)
	m.Get(api.Objs["permission"].URL, PermissionListHandler)
	m.Get("/watch"+api.Objs["permission"].URL, PermissionsWatchHandler)

	m.Post("/test"+api.Objs["role"].URL, RoleCreateHandler)
	m.Post(api.Objs["role"].URL, RoleCreateHandler)
	m.Put(api.Objs["role"].URL+"/:name", RoleCreateHandler)
	m.Delete("/test"+api.Objs["role"].URL+"/:name", RoleTestDeleteHandler)
	m.Delete(api.Objs["role"].URL+"/:name", RoleActualDeleteHandler)
	m.Get(api.Objs["role"].URL+"/:name", RoleGetHandler)
	m.Get(api.Objs["role"].URL, RoleListHandler)
	m.Get("/watch"+api.Objs["role"].URL, RolesWatchHandler)

	m.Post("/test"+api.Objs["securityGroup"].URL, SecurityGroupCreateHandler)
	m.Post(api.Objs["securityGroup"].URL, SecurityGroupCreateHandler)
	m.Put(api.Objs["securityGroup"].URL+"/:name", SecurityGroupCreateHandler)
	m.Delete("/test"+api.Objs["securityGroup"].URL+"/:name", SecurityGroupTestDeleteHandler)
	m.Delete(api.Objs["securityGroup"].URL+"/:name", SecurityGroupActualDeleteHandler)
	m.Get(api.Objs["securityGroup"].URL+"/:name", SecurityGroupGetHandler)
	m.Get(api.Objs["securityGroup"].URL, SecurityGroupListHandler)
	m.Get("/watch"+api.Objs["securityGroup"].URL, SecurityGroupsWatchHandler)

	m.Post("/test"+api.Objs["service"].URL, ServiceCreateHandler)
	m.Post(api.Objs["service"].URL, ServiceCreateHandler)
	m.Put(api.Objs["service"].URL+"/:name", ServiceCreateHandler)
	m.Delete("/test"+api.Objs["service"].URL+"/:name", ServiceTestDeleteHandler)
	m.Delete(api.Objs["service"].URL+"/:name", ServiceActualDeleteHandler)
	m.Get(api.Objs["service"].URL+"/:name", ServiceGetHandler)
	m.Get(api.Objs["service"].URL, ServiceListHandler)
	m.Get("/watch"+api.Objs["service"].URL, ServicesWatchHandler)

	m.Post("/test"+api.Objs["sgpolicy"].URL, SgpolicyCreateHandler)
	m.Post(api.Objs["sgpolicy"].URL, SgpolicyCreateHandler)
	m.Put(api.Objs["sgpolicy"].URL+"/:name", SgpolicyCreateHandler)
	m.Delete("/test"+api.Objs["sgpolicy"].URL+"/:name", SgpolicyTestDeleteHandler)
	m.Delete(api.Objs["sgpolicy"].URL+"/:name", SgpolicyActualDeleteHandler)
	m.Get(api.Objs["sgpolicy"].URL+"/:name", SgpolicyGetHandler)
	m.Get(api.Objs["sgpolicy"].URL, SgpolicyListHandler)
	m.Get("/watch"+api.Objs["sgpolicy"].URL, SgpolicysWatchHandler)

	m.Post("/test"+api.Objs["tenant"].URL, TenantCreateHandler)
	m.Post(api.Objs["tenant"].URL, TenantCreateHandler)
	m.Put(api.Objs["tenant"].URL+"/:name", TenantCreateHandler)
	m.Delete("/test"+api.Objs["tenant"].URL+"/:name", TenantTestDeleteHandler)
	m.Delete(api.Objs["tenant"].URL+"/:name", TenantActualDeleteHandler)
	m.Get(api.Objs["tenant"].URL+"/:name", TenantGetHandler)
	m.Get(api.Objs["tenant"].URL, TenantListHandler)
	m.Get("/watch"+api.Objs["tenant"].URL, TenantsWatchHandler)

	m.Post("/test"+api.Objs["user"].URL, UserCreateHandler)
	m.Post(api.Objs["user"].URL, UserCreateHandler)
	m.Put(api.Objs["user"].URL+"/:name", UserCreateHandler)
	m.Delete("/test"+api.Objs["user"].URL+"/:name", UserTestDeleteHandler)
	m.Delete(api.Objs["user"].URL+"/:name", UserActualDeleteHandler)
	m.Get(api.Objs["user"].URL+"/:name", UserGetHandler)
	m.Get(api.Objs["user"].URL, UserListHandler)
	m.Get("/watch"+api.Objs["user"].URL, UsersWatchHandler)

	return m
}

// ClusterCreateHandler creates a cluster.
func ClusterCreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	clusterObj := cmd.Cluster{}
	oldcluster := cmd.Cluster{}
	if err := decoder.Decode(&clusterObj); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	dryRun := false
	if strings.HasPrefix(req.URL.Path, "/test") {
		req.URL.Path = strings.TrimPrefix(req.URL.Path, "/test")
		dryRun = true
	}

	objName := clusterObj.Name
	objUUID := clusterObj.UUID
	if objName != "" {
		v, err := findUUIDByName("cluster", objName)
		if err == nil {
			if objUUID != "" && v != objUUID {
				log.Infof("name '%s' is already used by uuid '%s'\n", v)
				return http.StatusNotFound, fmt.Sprintf("name %s already in use", objName)
			}
			objUUID = v
		}
	}

	if objUUID == "" {
		if objName == "" {
			return http.StatusNotFound, fmt.Sprintf("Either a name or UUID must be specified")
		}
		objUUID = uuid.NewV4().String()
		clusterObj.UUID = objUUID
	}

	update := false
	key := path.Join(api.Objs["cluster"].URL, objUUID)
	if err := kvStore.Get(context.Background(), key, &oldcluster); err == nil {
		clusterObj.Status = oldcluster.Status
		clusterObj.UUID = oldcluster.UUID
		update = true
	}

	if err := PreCreateCallback(&clusterObj, update, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}
	log.Infof("Create key: %s object %+v", key, clusterObj)

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Cluster %q creation would be successful", clusterObj.Name)
	}

	if update {
		if err := kvStore.Update(context.Background(), key, &clusterObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	} else {
		if err := kvStore.Create(context.Background(), key, &clusterObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	}
	saveNameUUID(clusterObj.Kind, objName, objUUID)

	return http.StatusOK, "{}"
}

// ClusterTestDeleteHandler is
func ClusterTestDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return ClusterDeleteHandler(w, params, true)
}

// ClusterActualDeleteHandler is
func ClusterActualDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return ClusterDeleteHandler(w, params, false)
}

// ClusterDeleteHandler is
func ClusterDeleteHandler(w http.ResponseWriter, params martini.Params, dryRun bool) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("cluster", objName)
		if err != nil {
			return http.StatusNotFound, fmt.Sprintf("object %s not found", objName)
		}
		objUUID = v
	}

	key := path.Join(api.Objs["cluster"].URL, objUUID)
	cluster := cmd.Cluster{}
	if err := kvStore.Get(context.Background(), key, &cluster); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Cluster %q deletion failed: %v\n", objName, err)
	}

	if err := PreDeleteCallback(&cluster, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Cluster %q creation would be successful", cluster.Name)
	}

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Cluster %q deletion failed: %v\n", objName, err)
	}
	clearNameUUID("cluster", objName, objUUID)

	out, err := json.Marshal(&cluster)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// ClusterGetHandler looks up a cluster.
func ClusterGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("cluster", objName)
		if err != nil {
			return http.StatusNotFound, ""
		}
		objUUID = v
	}

	key := path.Join(api.Objs["cluster"].URL, objUUID)

	cluster := cmd.Cluster{}

	if err := kvStore.Get(context.Background(), key, &cluster); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Cluster %q not found\n", objName)
		}
		return http.StatusInternalServerError, fmt.Sprintf("Cluster %q get failed with error: %v\n", objName, err)
	}

	out, err := json.Marshal(&cluster)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// ClusterListHandler lists all clusters.
func ClusterListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	clusters := cmd.ClusterList{}

	// FIXME: URL tenant messup bug
	url := api.Objs["cluster"].URL
	url = strings.Replace(url, "//", "/", -1)
	if err := kvStore.List(context.Background(), url, &clusters); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Clusters not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("Clusters list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&clusters)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}

	return http.StatusOK, string(out)
}

// ClustersWatchHandler establishes a watch on clusters hierarchy.
func ClustersWatchHandler(w http.ResponseWriter, req *http.Request) {
	notifier, ok := w.(http.CloseNotifier)
	if !ok {
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		return
	}
	encoder := json.NewEncoder(w)
	watcher, err := kvStore.PrefixWatch(context.Background(), api.Objs["cluster"].URL, "0")
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

// EndpointCreateHandler creates a endpoint.
func EndpointCreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	endpointObj := network.Endpoint{}
	oldendpoint := network.Endpoint{}
	if err := decoder.Decode(&endpointObj); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	dryRun := false
	if strings.HasPrefix(req.URL.Path, "/test") {
		req.URL.Path = strings.TrimPrefix(req.URL.Path, "/test")
		dryRun = true
	}

	objName := endpointObj.Name
	objUUID := endpointObj.UUID
	if objName != "" {
		v, err := findUUIDByName("endpoint", objName)
		if err == nil {
			if objUUID != "" && v != objUUID {
				log.Infof("name '%s' is already used by uuid '%s'\n", v)
				return http.StatusNotFound, fmt.Sprintf("name %s already in use", objName)
			}
			objUUID = v
		}
	}

	if objUUID == "" {
		if objName == "" {
			return http.StatusNotFound, fmt.Sprintf("Either a name or UUID must be specified")
		}
		objUUID = uuid.NewV4().String()
		endpointObj.UUID = objUUID
	}

	update := false
	key := path.Join(api.Objs["endpoint"].URL, objUUID)
	if err := kvStore.Get(context.Background(), key, &oldendpoint); err == nil {
		endpointObj.Status = oldendpoint.Status
		endpointObj.UUID = oldendpoint.UUID
		update = true
	}

	if err := PreCreateCallback(&endpointObj, update, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}
	log.Infof("Create key: %s object %+v", key, endpointObj)

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Endpoint %q creation would be successful", endpointObj.Name)
	}

	if update {
		if err := kvStore.Update(context.Background(), key, &endpointObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	} else {
		if err := kvStore.Create(context.Background(), key, &endpointObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	}
	saveNameUUID(endpointObj.Kind, objName, objUUID)

	return http.StatusOK, "{}"
}

// EndpointTestDeleteHandler is
func EndpointTestDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return EndpointDeleteHandler(w, params, true)
}

// EndpointActualDeleteHandler is
func EndpointActualDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return EndpointDeleteHandler(w, params, false)
}

// EndpointDeleteHandler is
func EndpointDeleteHandler(w http.ResponseWriter, params martini.Params, dryRun bool) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("endpoint", objName)
		if err != nil {
			return http.StatusNotFound, fmt.Sprintf("object %s not found", objName)
		}
		objUUID = v
	}

	key := path.Join(api.Objs["endpoint"].URL, objUUID)
	endpoint := network.Endpoint{}
	if err := kvStore.Get(context.Background(), key, &endpoint); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Endpoint %q deletion failed: %v\n", objName, err)
	}

	if err := PreDeleteCallback(&endpoint, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Endpoint %q creation would be successful", endpoint.Name)
	}

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Endpoint %q deletion failed: %v\n", objName, err)
	}
	clearNameUUID("endpoint", objName, objUUID)

	out, err := json.Marshal(&endpoint)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// EndpointGetHandler looks up a endpoint.
func EndpointGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("endpoint", objName)
		if err != nil {
			return http.StatusNotFound, ""
		}
		objUUID = v
	}

	key := path.Join(api.Objs["endpoint"].URL, objUUID)

	endpoint := network.Endpoint{}

	if err := kvStore.Get(context.Background(), key, &endpoint); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Endpoint %q not found\n", objName)
		}
		return http.StatusInternalServerError, fmt.Sprintf("Endpoint %q get failed with error: %v\n", objName, err)
	}

	out, err := json.Marshal(&endpoint)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// EndpointListHandler lists all endpoints.
func EndpointListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	endpoints := network.EndpointList{}

	// FIXME: URL tenant messup bug
	url := api.Objs["endpoint"].URL
	url = strings.Replace(url, "//", "/", -1)
	if err := kvStore.List(context.Background(), url, &endpoints); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Endpoints not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("Endpoints list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&endpoints)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}

	return http.StatusOK, string(out)
}

// EndpointsWatchHandler establishes a watch on endpoints hierarchy.
func EndpointsWatchHandler(w http.ResponseWriter, req *http.Request) {
	notifier, ok := w.(http.CloseNotifier)
	if !ok {
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		return
	}
	encoder := json.NewEncoder(w)
	watcher, err := kvStore.PrefixWatch(context.Background(), api.Objs["endpoint"].URL, "0")
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

// LbPolicyCreateHandler creates a lbPolicy.
func LbPolicyCreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	lbPolicyObj := network.LbPolicy{}
	oldlbPolicy := network.LbPolicy{}
	if err := decoder.Decode(&lbPolicyObj); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	dryRun := false
	if strings.HasPrefix(req.URL.Path, "/test") {
		req.URL.Path = strings.TrimPrefix(req.URL.Path, "/test")
		dryRun = true
	}

	objName := lbPolicyObj.Name
	objUUID := lbPolicyObj.UUID
	if objName != "" {
		v, err := findUUIDByName("lbPolicy", objName)
		if err == nil {
			if objUUID != "" && v != objUUID {
				log.Infof("name '%s' is already used by uuid '%s'\n", v)
				return http.StatusNotFound, fmt.Sprintf("name %s already in use", objName)
			}
			objUUID = v
		}
	}

	if objUUID == "" {
		if objName == "" {
			return http.StatusNotFound, fmt.Sprintf("Either a name or UUID must be specified")
		}
		objUUID = uuid.NewV4().String()
		lbPolicyObj.UUID = objUUID
	}

	update := false
	key := path.Join(api.Objs["lbPolicy"].URL, objUUID)
	if err := kvStore.Get(context.Background(), key, &oldlbPolicy); err == nil {
		lbPolicyObj.Status = oldlbPolicy.Status
		lbPolicyObj.UUID = oldlbPolicy.UUID
		update = true
	}

	if err := PreCreateCallback(&lbPolicyObj, update, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}
	log.Infof("Create key: %s object %+v", key, lbPolicyObj)

	if dryRun {
		return http.StatusOK, fmt.Sprintf("LbPolicy %q creation would be successful", lbPolicyObj.Name)
	}

	if update {
		if err := kvStore.Update(context.Background(), key, &lbPolicyObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	} else {
		if err := kvStore.Create(context.Background(), key, &lbPolicyObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	}
	saveNameUUID(lbPolicyObj.Kind, objName, objUUID)

	return http.StatusOK, "{}"
}

// LbPolicyTestDeleteHandler is
func LbPolicyTestDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return LbPolicyDeleteHandler(w, params, true)
}

// LbPolicyActualDeleteHandler is
func LbPolicyActualDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return LbPolicyDeleteHandler(w, params, false)
}

// LbPolicyDeleteHandler is
func LbPolicyDeleteHandler(w http.ResponseWriter, params martini.Params, dryRun bool) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("lbPolicy", objName)
		if err != nil {
			return http.StatusNotFound, fmt.Sprintf("object %s not found", objName)
		}
		objUUID = v
	}

	key := path.Join(api.Objs["lbPolicy"].URL, objUUID)
	lbPolicy := network.LbPolicy{}
	if err := kvStore.Get(context.Background(), key, &lbPolicy); err != nil {
		return http.StatusNotFound, fmt.Sprintf("LbPolicy %q deletion failed: %v\n", objName, err)
	}

	if err := PreDeleteCallback(&lbPolicy, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	if dryRun {
		return http.StatusOK, fmt.Sprintf("LbPolicy %q creation would be successful", lbPolicy.Name)
	}

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("LbPolicy %q deletion failed: %v\n", objName, err)
	}
	clearNameUUID("lbPolicy", objName, objUUID)

	out, err := json.Marshal(&lbPolicy)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// LbPolicyGetHandler looks up a lbPolicy.
func LbPolicyGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("lbPolicy", objName)
		if err != nil {
			return http.StatusNotFound, ""
		}
		objUUID = v
	}

	key := path.Join(api.Objs["lbPolicy"].URL, objUUID)

	lbPolicy := network.LbPolicy{}

	if err := kvStore.Get(context.Background(), key, &lbPolicy); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("LbPolicy %q not found\n", objName)
		}
		return http.StatusInternalServerError, fmt.Sprintf("LbPolicy %q get failed with error: %v\n", objName, err)
	}

	out, err := json.Marshal(&lbPolicy)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// LbPolicyListHandler lists all lbPolicys.
func LbPolicyListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	lbPolicys := network.LbPolicyList{}

	// FIXME: URL tenant messup bug
	url := api.Objs["lbPolicy"].URL
	url = strings.Replace(url, "//", "/", -1)
	if err := kvStore.List(context.Background(), url, &lbPolicys); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("LbPolicys not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("LbPolicys list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&lbPolicys)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}

	return http.StatusOK, string(out)
}

// LbPolicysWatchHandler establishes a watch on lbPolicys hierarchy.
func LbPolicysWatchHandler(w http.ResponseWriter, req *http.Request) {
	notifier, ok := w.(http.CloseNotifier)
	if !ok {
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		return
	}
	encoder := json.NewEncoder(w)
	watcher, err := kvStore.PrefixWatch(context.Background(), api.Objs["lbPolicy"].URL, "0")
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

// NetworkCreateHandler creates a network.
func NetworkCreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	networkObj := network.Network{}
	oldnetwork := network.Network{}
	if err := decoder.Decode(&networkObj); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	dryRun := false
	if strings.HasPrefix(req.URL.Path, "/test") {
		req.URL.Path = strings.TrimPrefix(req.URL.Path, "/test")
		dryRun = true
	}

	objName := networkObj.Name
	objUUID := networkObj.UUID
	if objName != "" {
		v, err := findUUIDByName("network", objName)
		if err == nil {
			if objUUID != "" && v != objUUID {
				log.Infof("name '%s' is already used by uuid '%s'\n", v)
				return http.StatusNotFound, fmt.Sprintf("name %s already in use", objName)
			}
			objUUID = v
		}
	}

	if objUUID == "" {
		if objName == "" {
			return http.StatusNotFound, fmt.Sprintf("Either a name or UUID must be specified")
		}
		objUUID = uuid.NewV4().String()
		networkObj.UUID = objUUID
	}

	update := false
	key := path.Join(api.Objs["network"].URL, objUUID)
	if err := kvStore.Get(context.Background(), key, &oldnetwork); err == nil {
		networkObj.Status = oldnetwork.Status
		networkObj.UUID = oldnetwork.UUID
		update = true
	}

	if err := PreCreateCallback(&networkObj, update, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}
	log.Infof("Create key: %s object %+v", key, networkObj)

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Network %q creation would be successful", networkObj.Name)
	}

	if update {
		if err := kvStore.Update(context.Background(), key, &networkObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	} else {
		if err := kvStore.Create(context.Background(), key, &networkObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	}
	saveNameUUID(networkObj.Kind, objName, objUUID)

	return http.StatusOK, "{}"
}

// NetworkTestDeleteHandler is
func NetworkTestDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return NetworkDeleteHandler(w, params, true)
}

// NetworkActualDeleteHandler is
func NetworkActualDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return NetworkDeleteHandler(w, params, false)
}

// NetworkDeleteHandler is
func NetworkDeleteHandler(w http.ResponseWriter, params martini.Params, dryRun bool) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("network", objName)
		if err != nil {
			return http.StatusNotFound, fmt.Sprintf("object %s not found", objName)
		}
		objUUID = v
	}

	key := path.Join(api.Objs["network"].URL, objUUID)
	network := network.Network{}
	if err := kvStore.Get(context.Background(), key, &network); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Network %q deletion failed: %v\n", objName, err)
	}

	if err := PreDeleteCallback(&network, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Network %q creation would be successful", network.Name)
	}

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Network %q deletion failed: %v\n", objName, err)
	}
	clearNameUUID("network", objName, objUUID)

	out, err := json.Marshal(&network)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// NetworkGetHandler looks up a network.
func NetworkGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("network", objName)
		if err != nil {
			return http.StatusNotFound, ""
		}
		objUUID = v
	}

	key := path.Join(api.Objs["network"].URL, objUUID)

	network := network.Network{}

	if err := kvStore.Get(context.Background(), key, &network); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Network %q not found\n", objName)
		}
		return http.StatusInternalServerError, fmt.Sprintf("Network %q get failed with error: %v\n", objName, err)
	}

	out, err := json.Marshal(&network)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// NetworkListHandler lists all networks.
func NetworkListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	networks := network.NetworkList{}

	// FIXME: URL tenant messup bug
	url := api.Objs["network"].URL
	url = strings.Replace(url, "//", "/", -1)
	if err := kvStore.List(context.Background(), url, &networks); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Networks not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("Networks list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&networks)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}

	return http.StatusOK, string(out)
}

// NetworksWatchHandler establishes a watch on networks hierarchy.
func NetworksWatchHandler(w http.ResponseWriter, req *http.Request) {
	notifier, ok := w.(http.CloseNotifier)
	if !ok {
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		return
	}
	encoder := json.NewEncoder(w)
	watcher, err := kvStore.PrefixWatch(context.Background(), api.Objs["network"].URL, "0")
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

// NodeCreateHandler creates a node.
func NodeCreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	nodeObj := cmd.Node{}
	oldnode := cmd.Node{}
	if err := decoder.Decode(&nodeObj); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	dryRun := false
	if strings.HasPrefix(req.URL.Path, "/test") {
		req.URL.Path = strings.TrimPrefix(req.URL.Path, "/test")
		dryRun = true
	}

	objName := nodeObj.Name
	objUUID := nodeObj.UUID
	if objName != "" {
		v, err := findUUIDByName("node", objName)
		if err == nil {
			if objUUID != "" && v != objUUID {
				log.Infof("name '%s' is already used by uuid '%s'\n", v)
				return http.StatusNotFound, fmt.Sprintf("name %s already in use", objName)
			}
			objUUID = v
		}
	}

	if objUUID == "" {
		if objName == "" {
			return http.StatusNotFound, fmt.Sprintf("Either a name or UUID must be specified")
		}
		objUUID = uuid.NewV4().String()
		nodeObj.UUID = objUUID
	}

	update := false
	key := path.Join(api.Objs["node"].URL, objUUID)
	if err := kvStore.Get(context.Background(), key, &oldnode); err == nil {
		nodeObj.Status = oldnode.Status
		nodeObj.UUID = oldnode.UUID
		update = true
	}

	if err := PreCreateCallback(&nodeObj, update, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}
	log.Infof("Create key: %s object %+v", key, nodeObj)

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Node %q creation would be successful", nodeObj.Name)
	}

	if update {
		if err := kvStore.Update(context.Background(), key, &nodeObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	} else {
		if err := kvStore.Create(context.Background(), key, &nodeObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	}
	saveNameUUID(nodeObj.Kind, objName, objUUID)

	return http.StatusOK, "{}"
}

// NodeTestDeleteHandler is
func NodeTestDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return NodeDeleteHandler(w, params, true)
}

// NodeActualDeleteHandler is
func NodeActualDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return NodeDeleteHandler(w, params, false)
}

// NodeDeleteHandler is
func NodeDeleteHandler(w http.ResponseWriter, params martini.Params, dryRun bool) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("node", objName)
		if err != nil {
			return http.StatusNotFound, fmt.Sprintf("object %s not found", objName)
		}
		objUUID = v
	}

	key := path.Join(api.Objs["node"].URL, objUUID)
	node := cmd.Node{}
	if err := kvStore.Get(context.Background(), key, &node); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Node %q deletion failed: %v\n", objName, err)
	}

	if err := PreDeleteCallback(&node, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Node %q creation would be successful", node.Name)
	}

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Node %q deletion failed: %v\n", objName, err)
	}
	clearNameUUID("node", objName, objUUID)

	out, err := json.Marshal(&node)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// NodeGetHandler looks up a node.
func NodeGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("node", objName)
		if err != nil {
			return http.StatusNotFound, ""
		}
		objUUID = v
	}

	key := path.Join(api.Objs["node"].URL, objUUID)

	node := cmd.Node{}

	if err := kvStore.Get(context.Background(), key, &node); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Node %q not found\n", objName)
		}
		return http.StatusInternalServerError, fmt.Sprintf("Node %q get failed with error: %v\n", objName, err)
	}

	out, err := json.Marshal(&node)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// NodeListHandler lists all nodes.
func NodeListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	nodes := cmd.NodeList{}

	// FIXME: URL tenant messup bug
	url := api.Objs["node"].URL
	url = strings.Replace(url, "//", "/", -1)
	if err := kvStore.List(context.Background(), url, &nodes); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Nodes not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("Nodes list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&nodes)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}

	return http.StatusOK, string(out)
}

// NodesWatchHandler establishes a watch on nodes hierarchy.
func NodesWatchHandler(w http.ResponseWriter, req *http.Request) {
	notifier, ok := w.(http.CloseNotifier)
	if !ok {
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		return
	}
	encoder := json.NewEncoder(w)
	watcher, err := kvStore.PrefixWatch(context.Background(), api.Objs["node"].URL, "0")
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

// PermissionCreateHandler creates a permission.
func PermissionCreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	permissionObj := api.Permission{}
	oldpermission := api.Permission{}
	if err := decoder.Decode(&permissionObj); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	dryRun := false
	if strings.HasPrefix(req.URL.Path, "/test") {
		req.URL.Path = strings.TrimPrefix(req.URL.Path, "/test")
		dryRun = true
	}

	objName := permissionObj.Name
	objUUID := permissionObj.UUID
	if objName != "" {
		v, err := findUUIDByName("permission", objName)
		if err == nil {
			if objUUID != "" && v != objUUID {
				log.Infof("name '%s' is already used by uuid '%s'\n", v)
				return http.StatusNotFound, fmt.Sprintf("name %s already in use", objName)
			}
			objUUID = v
		}
	}

	if objUUID == "" {
		if objName == "" {
			return http.StatusNotFound, fmt.Sprintf("Either a name or UUID must be specified")
		}
		objUUID = uuid.NewV4().String()
		permissionObj.UUID = objUUID
	}

	update := false
	key := path.Join(api.Objs["permission"].URL, objUUID)
	if err := kvStore.Get(context.Background(), key, &oldpermission); err == nil {
		permissionObj.Status = oldpermission.Status
		permissionObj.UUID = oldpermission.UUID
		update = true
	}

	if err := PreCreateCallback(&permissionObj, update, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}
	log.Infof("Create key: %s object %+v", key, permissionObj)

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Permission %q creation would be successful", permissionObj.Name)
	}

	if update {
		if err := kvStore.Update(context.Background(), key, &permissionObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	} else {
		if err := kvStore.Create(context.Background(), key, &permissionObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	}
	saveNameUUID(permissionObj.Kind, objName, objUUID)

	return http.StatusOK, "{}"
}

// PermissionTestDeleteHandler is
func PermissionTestDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return PermissionDeleteHandler(w, params, true)
}

// PermissionActualDeleteHandler is
func PermissionActualDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return PermissionDeleteHandler(w, params, false)
}

// PermissionDeleteHandler is
func PermissionDeleteHandler(w http.ResponseWriter, params martini.Params, dryRun bool) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("permission", objName)
		if err != nil {
			return http.StatusNotFound, fmt.Sprintf("object %s not found", objName)
		}
		objUUID = v
	}

	key := path.Join(api.Objs["permission"].URL, objUUID)
	permission := api.Permission{}
	if err := kvStore.Get(context.Background(), key, &permission); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Permission %q deletion failed: %v\n", objName, err)
	}

	if err := PreDeleteCallback(&permission, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Permission %q creation would be successful", permission.Name)
	}

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Permission %q deletion failed: %v\n", objName, err)
	}
	clearNameUUID("permission", objName, objUUID)

	out, err := json.Marshal(&permission)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// PermissionGetHandler looks up a permission.
func PermissionGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("permission", objName)
		if err != nil {
			return http.StatusNotFound, ""
		}
		objUUID = v
	}

	key := path.Join(api.Objs["permission"].URL, objUUID)

	permission := api.Permission{}

	if err := kvStore.Get(context.Background(), key, &permission); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Permission %q not found\n", objName)
		}
		return http.StatusInternalServerError, fmt.Sprintf("Permission %q get failed with error: %v\n", objName, err)
	}

	out, err := json.Marshal(&permission)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// PermissionListHandler lists all permissions.
func PermissionListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	permissions := api.PermissionList{}

	// FIXME: URL tenant messup bug
	url := api.Objs["permission"].URL
	url = strings.Replace(url, "//", "/", -1)
	if err := kvStore.List(context.Background(), url, &permissions); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Permissions not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("Permissions list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&permissions)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}

	return http.StatusOK, string(out)
}

// PermissionsWatchHandler establishes a watch on permissions hierarchy.
func PermissionsWatchHandler(w http.ResponseWriter, req *http.Request) {
	notifier, ok := w.(http.CloseNotifier)
	if !ok {
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		return
	}
	encoder := json.NewEncoder(w)
	watcher, err := kvStore.PrefixWatch(context.Background(), api.Objs["permission"].URL, "0")
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

// RoleCreateHandler creates a role.
func RoleCreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	roleObj := api.Role{}
	oldrole := api.Role{}
	if err := decoder.Decode(&roleObj); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	dryRun := false
	if strings.HasPrefix(req.URL.Path, "/test") {
		req.URL.Path = strings.TrimPrefix(req.URL.Path, "/test")
		dryRun = true
	}

	objName := roleObj.Name
	objUUID := roleObj.UUID
	if objName != "" {
		v, err := findUUIDByName("role", objName)
		if err == nil {
			if objUUID != "" && v != objUUID {
				log.Infof("name '%s' is already used by uuid '%s'\n", v)
				return http.StatusNotFound, fmt.Sprintf("name %s already in use", objName)
			}
			objUUID = v
		}
	}

	if objUUID == "" {
		if objName == "" {
			return http.StatusNotFound, fmt.Sprintf("Either a name or UUID must be specified")
		}
		objUUID = uuid.NewV4().String()
		roleObj.UUID = objUUID
	}

	update := false
	key := path.Join(api.Objs["role"].URL, objUUID)
	if err := kvStore.Get(context.Background(), key, &oldrole); err == nil {
		roleObj.Status = oldrole.Status
		roleObj.UUID = oldrole.UUID
		update = true
	}

	if err := PreCreateCallback(&roleObj, update, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}
	log.Infof("Create key: %s object %+v", key, roleObj)

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Role %q creation would be successful", roleObj.Name)
	}

	if update {
		if err := kvStore.Update(context.Background(), key, &roleObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	} else {
		if err := kvStore.Create(context.Background(), key, &roleObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	}
	saveNameUUID(roleObj.Kind, objName, objUUID)

	return http.StatusOK, "{}"
}

// RoleTestDeleteHandler is
func RoleTestDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return RoleDeleteHandler(w, params, true)
}

// RoleActualDeleteHandler is
func RoleActualDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return RoleDeleteHandler(w, params, false)
}

// RoleDeleteHandler is
func RoleDeleteHandler(w http.ResponseWriter, params martini.Params, dryRun bool) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("role", objName)
		if err != nil {
			return http.StatusNotFound, fmt.Sprintf("object %s not found", objName)
		}
		objUUID = v
	}

	key := path.Join(api.Objs["role"].URL, objUUID)
	role := api.Role{}
	if err := kvStore.Get(context.Background(), key, &role); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Role %q deletion failed: %v\n", objName, err)
	}

	if err := PreDeleteCallback(&role, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Role %q creation would be successful", role.Name)
	}

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Role %q deletion failed: %v\n", objName, err)
	}
	clearNameUUID("role", objName, objUUID)

	out, err := json.Marshal(&role)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// RoleGetHandler looks up a role.
func RoleGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("role", objName)
		if err != nil {
			return http.StatusNotFound, ""
		}
		objUUID = v
	}

	key := path.Join(api.Objs["role"].URL, objUUID)

	role := api.Role{}

	if err := kvStore.Get(context.Background(), key, &role); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Role %q not found\n", objName)
		}
		return http.StatusInternalServerError, fmt.Sprintf("Role %q get failed with error: %v\n", objName, err)
	}

	out, err := json.Marshal(&role)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// RoleListHandler lists all roles.
func RoleListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	roles := api.RoleList{}

	// FIXME: URL tenant messup bug
	url := api.Objs["role"].URL
	url = strings.Replace(url, "//", "/", -1)
	if err := kvStore.List(context.Background(), url, &roles); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Roles not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("Roles list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&roles)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}

	return http.StatusOK, string(out)
}

// RolesWatchHandler establishes a watch on roles hierarchy.
func RolesWatchHandler(w http.ResponseWriter, req *http.Request) {
	notifier, ok := w.(http.CloseNotifier)
	if !ok {
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		return
	}
	encoder := json.NewEncoder(w)
	watcher, err := kvStore.PrefixWatch(context.Background(), api.Objs["role"].URL, "0")
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

// SecurityGroupCreateHandler creates a securityGroup.
func SecurityGroupCreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	securityGroupObj := network.SecurityGroup{}
	oldsecurityGroup := network.SecurityGroup{}
	if err := decoder.Decode(&securityGroupObj); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	dryRun := false
	if strings.HasPrefix(req.URL.Path, "/test") {
		req.URL.Path = strings.TrimPrefix(req.URL.Path, "/test")
		dryRun = true
	}

	objName := securityGroupObj.Name
	objUUID := securityGroupObj.UUID
	if objName != "" {
		v, err := findUUIDByName("securityGroup", objName)
		if err == nil {
			if objUUID != "" && v != objUUID {
				log.Infof("name '%s' is already used by uuid '%s'\n", v)
				return http.StatusNotFound, fmt.Sprintf("name %s already in use", objName)
			}
			objUUID = v
		}
	}

	if objUUID == "" {
		if objName == "" {
			return http.StatusNotFound, fmt.Sprintf("Either a name or UUID must be specified")
		}
		objUUID = uuid.NewV4().String()
		securityGroupObj.UUID = objUUID
	}

	update := false
	key := path.Join(api.Objs["securityGroup"].URL, objUUID)
	if err := kvStore.Get(context.Background(), key, &oldsecurityGroup); err == nil {
		securityGroupObj.Status = oldsecurityGroup.Status
		securityGroupObj.UUID = oldsecurityGroup.UUID
		update = true
	}

	if err := PreCreateCallback(&securityGroupObj, update, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}
	log.Infof("Create key: %s object %+v", key, securityGroupObj)

	if dryRun {
		return http.StatusOK, fmt.Sprintf("SecurityGroup %q creation would be successful", securityGroupObj.Name)
	}

	if update {
		if err := kvStore.Update(context.Background(), key, &securityGroupObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	} else {
		if err := kvStore.Create(context.Background(), key, &securityGroupObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	}
	saveNameUUID(securityGroupObj.Kind, objName, objUUID)

	return http.StatusOK, "{}"
}

// SecurityGroupTestDeleteHandler is
func SecurityGroupTestDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return SecurityGroupDeleteHandler(w, params, true)
}

// SecurityGroupActualDeleteHandler is
func SecurityGroupActualDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return SecurityGroupDeleteHandler(w, params, false)
}

// SecurityGroupDeleteHandler is
func SecurityGroupDeleteHandler(w http.ResponseWriter, params martini.Params, dryRun bool) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("securityGroup", objName)
		if err != nil {
			return http.StatusNotFound, fmt.Sprintf("object %s not found", objName)
		}
		objUUID = v
	}

	key := path.Join(api.Objs["securityGroup"].URL, objUUID)
	securityGroup := network.SecurityGroup{}
	if err := kvStore.Get(context.Background(), key, &securityGroup); err != nil {
		return http.StatusNotFound, fmt.Sprintf("SecurityGroup %q deletion failed: %v\n", objName, err)
	}

	if err := PreDeleteCallback(&securityGroup, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	if dryRun {
		return http.StatusOK, fmt.Sprintf("SecurityGroup %q creation would be successful", securityGroup.Name)
	}

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("SecurityGroup %q deletion failed: %v\n", objName, err)
	}
	clearNameUUID("securityGroup", objName, objUUID)

	out, err := json.Marshal(&securityGroup)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// SecurityGroupGetHandler looks up a securityGroup.
func SecurityGroupGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("securityGroup", objName)
		if err != nil {
			return http.StatusNotFound, ""
		}
		objUUID = v
	}

	key := path.Join(api.Objs["securityGroup"].URL, objUUID)

	securityGroup := network.SecurityGroup{}

	if err := kvStore.Get(context.Background(), key, &securityGroup); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("SecurityGroup %q not found\n", objName)
		}
		return http.StatusInternalServerError, fmt.Sprintf("SecurityGroup %q get failed with error: %v\n", objName, err)
	}

	out, err := json.Marshal(&securityGroup)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// SecurityGroupListHandler lists all securityGroups.
func SecurityGroupListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	securityGroups := network.SecurityGroupList{}

	// FIXME: URL tenant messup bug
	url := api.Objs["securityGroup"].URL
	url = strings.Replace(url, "//", "/", -1)
	if err := kvStore.List(context.Background(), url, &securityGroups); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("SecurityGroups not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("SecurityGroups list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&securityGroups)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}

	return http.StatusOK, string(out)
}

// SecurityGroupsWatchHandler establishes a watch on securityGroups hierarchy.
func SecurityGroupsWatchHandler(w http.ResponseWriter, req *http.Request) {
	notifier, ok := w.(http.CloseNotifier)
	if !ok {
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		return
	}
	encoder := json.NewEncoder(w)
	watcher, err := kvStore.PrefixWatch(context.Background(), api.Objs["securityGroup"].URL, "0")
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

// ServiceCreateHandler creates a service.
func ServiceCreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	serviceObj := network.Service{}
	oldservice := network.Service{}
	if err := decoder.Decode(&serviceObj); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	dryRun := false
	if strings.HasPrefix(req.URL.Path, "/test") {
		req.URL.Path = strings.TrimPrefix(req.URL.Path, "/test")
		dryRun = true
	}

	objName := serviceObj.Name
	objUUID := serviceObj.UUID
	if objName != "" {
		v, err := findUUIDByName("service", objName)
		if err == nil {
			if objUUID != "" && v != objUUID {
				log.Infof("name '%s' is already used by uuid '%s'\n", v)
				return http.StatusNotFound, fmt.Sprintf("name %s already in use", objName)
			}
			objUUID = v
		}
	}

	if objUUID == "" {
		if objName == "" {
			return http.StatusNotFound, fmt.Sprintf("Either a name or UUID must be specified")
		}
		objUUID = uuid.NewV4().String()
		serviceObj.UUID = objUUID
	}

	update := false
	key := path.Join(api.Objs["service"].URL, objUUID)
	if err := kvStore.Get(context.Background(), key, &oldservice); err == nil {
		serviceObj.Status = oldservice.Status
		serviceObj.UUID = oldservice.UUID
		update = true
	}

	if err := PreCreateCallback(&serviceObj, update, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}
	log.Infof("Create key: %s object %+v", key, serviceObj)

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Service %q creation would be successful", serviceObj.Name)
	}

	if update {
		if err := kvStore.Update(context.Background(), key, &serviceObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	} else {
		if err := kvStore.Create(context.Background(), key, &serviceObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	}
	saveNameUUID(serviceObj.Kind, objName, objUUID)

	return http.StatusOK, "{}"
}

// ServiceTestDeleteHandler is
func ServiceTestDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return ServiceDeleteHandler(w, params, true)
}

// ServiceActualDeleteHandler is
func ServiceActualDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return ServiceDeleteHandler(w, params, false)
}

// ServiceDeleteHandler is
func ServiceDeleteHandler(w http.ResponseWriter, params martini.Params, dryRun bool) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("service", objName)
		if err != nil {
			return http.StatusNotFound, fmt.Sprintf("object %s not found", objName)
		}
		objUUID = v
	}

	key := path.Join(api.Objs["service"].URL, objUUID)
	service := network.Service{}
	if err := kvStore.Get(context.Background(), key, &service); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Service %q deletion failed: %v\n", objName, err)
	}

	if err := PreDeleteCallback(&service, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Service %q creation would be successful", service.Name)
	}

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Service %q deletion failed: %v\n", objName, err)
	}
	clearNameUUID("service", objName, objUUID)

	out, err := json.Marshal(&service)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// ServiceGetHandler looks up a service.
func ServiceGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("service", objName)
		if err != nil {
			return http.StatusNotFound, ""
		}
		objUUID = v
	}

	key := path.Join(api.Objs["service"].URL, objUUID)

	service := network.Service{}

	if err := kvStore.Get(context.Background(), key, &service); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Service %q not found\n", objName)
		}
		return http.StatusInternalServerError, fmt.Sprintf("Service %q get failed with error: %v\n", objName, err)
	}

	out, err := json.Marshal(&service)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// ServiceListHandler lists all services.
func ServiceListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	services := network.ServiceList{}

	// FIXME: URL tenant messup bug
	url := api.Objs["service"].URL
	url = strings.Replace(url, "//", "/", -1)
	if err := kvStore.List(context.Background(), url, &services); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Services not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("Services list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&services)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}

	return http.StatusOK, string(out)
}

// ServicesWatchHandler establishes a watch on services hierarchy.
func ServicesWatchHandler(w http.ResponseWriter, req *http.Request) {
	notifier, ok := w.(http.CloseNotifier)
	if !ok {
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		return
	}
	encoder := json.NewEncoder(w)
	watcher, err := kvStore.PrefixWatch(context.Background(), api.Objs["service"].URL, "0")
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

// SgpolicyCreateHandler creates a sgpolicy.
func SgpolicyCreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	sgpolicyObj := network.Sgpolicy{}
	oldsgpolicy := network.Sgpolicy{}
	if err := decoder.Decode(&sgpolicyObj); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	dryRun := false
	if strings.HasPrefix(req.URL.Path, "/test") {
		req.URL.Path = strings.TrimPrefix(req.URL.Path, "/test")
		dryRun = true
	}

	objName := sgpolicyObj.Name
	objUUID := sgpolicyObj.UUID
	if objName != "" {
		v, err := findUUIDByName("sgpolicy", objName)
		if err == nil {
			if objUUID != "" && v != objUUID {
				log.Infof("name '%s' is already used by uuid '%s'\n", v)
				return http.StatusNotFound, fmt.Sprintf("name %s already in use", objName)
			}
			objUUID = v
		}
	}

	if objUUID == "" {
		if objName == "" {
			return http.StatusNotFound, fmt.Sprintf("Either a name or UUID must be specified")
		}
		objUUID = uuid.NewV4().String()
		sgpolicyObj.UUID = objUUID
	}

	update := false
	key := path.Join(api.Objs["sgpolicy"].URL, objUUID)
	if err := kvStore.Get(context.Background(), key, &oldsgpolicy); err == nil {
		sgpolicyObj.Status = oldsgpolicy.Status
		sgpolicyObj.UUID = oldsgpolicy.UUID
		update = true
	}

	if err := PreCreateCallback(&sgpolicyObj, update, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}
	log.Infof("Create key: %s object %+v", key, sgpolicyObj)

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Sgpolicy %q creation would be successful", sgpolicyObj.Name)
	}

	if update {
		if err := kvStore.Update(context.Background(), key, &sgpolicyObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	} else {
		if err := kvStore.Create(context.Background(), key, &sgpolicyObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	}
	saveNameUUID(sgpolicyObj.Kind, objName, objUUID)

	return http.StatusOK, "{}"
}

// SgpolicyTestDeleteHandler is
func SgpolicyTestDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return SgpolicyDeleteHandler(w, params, true)
}

// SgpolicyActualDeleteHandler is
func SgpolicyActualDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return SgpolicyDeleteHandler(w, params, false)
}

// SgpolicyDeleteHandler is
func SgpolicyDeleteHandler(w http.ResponseWriter, params martini.Params, dryRun bool) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("sgpolicy", objName)
		if err != nil {
			return http.StatusNotFound, fmt.Sprintf("object %s not found", objName)
		}
		objUUID = v
	}

	key := path.Join(api.Objs["sgpolicy"].URL, objUUID)
	sgpolicy := network.Sgpolicy{}
	if err := kvStore.Get(context.Background(), key, &sgpolicy); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Sgpolicy %q deletion failed: %v\n", objName, err)
	}

	if err := PreDeleteCallback(&sgpolicy, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Sgpolicy %q creation would be successful", sgpolicy.Name)
	}

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Sgpolicy %q deletion failed: %v\n", objName, err)
	}
	clearNameUUID("sgpolicy", objName, objUUID)

	out, err := json.Marshal(&sgpolicy)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// SgpolicyGetHandler looks up a sgpolicy.
func SgpolicyGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("sgpolicy", objName)
		if err != nil {
			return http.StatusNotFound, ""
		}
		objUUID = v
	}

	key := path.Join(api.Objs["sgpolicy"].URL, objUUID)

	sgpolicy := network.Sgpolicy{}

	if err := kvStore.Get(context.Background(), key, &sgpolicy); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Sgpolicy %q not found\n", objName)
		}
		return http.StatusInternalServerError, fmt.Sprintf("Sgpolicy %q get failed with error: %v\n", objName, err)
	}

	out, err := json.Marshal(&sgpolicy)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// SgpolicyListHandler lists all sgpolicys.
func SgpolicyListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	sgpolicys := network.SgpolicyList{}

	// FIXME: URL tenant messup bug
	url := api.Objs["sgpolicy"].URL
	url = strings.Replace(url, "//", "/", -1)
	if err := kvStore.List(context.Background(), url, &sgpolicys); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Sgpolicys not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("Sgpolicys list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&sgpolicys)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}

	return http.StatusOK, string(out)
}

// SgpolicysWatchHandler establishes a watch on sgpolicys hierarchy.
func SgpolicysWatchHandler(w http.ResponseWriter, req *http.Request) {
	notifier, ok := w.(http.CloseNotifier)
	if !ok {
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		return
	}
	encoder := json.NewEncoder(w)
	watcher, err := kvStore.PrefixWatch(context.Background(), api.Objs["sgpolicy"].URL, "0")
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

// TenantCreateHandler creates a tenant.
func TenantCreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	tenantObj := network.Tenant{}
	oldtenant := network.Tenant{}
	if err := decoder.Decode(&tenantObj); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	dryRun := false
	if strings.HasPrefix(req.URL.Path, "/test") {
		req.URL.Path = strings.TrimPrefix(req.URL.Path, "/test")
		dryRun = true
	}

	objName := tenantObj.Name
	objUUID := tenantObj.UUID
	if objName != "" {
		v, err := findUUIDByName("tenant", objName)
		if err == nil {
			if objUUID != "" && v != objUUID {
				log.Infof("name '%s' is already used by uuid '%s'\n", v)
				return http.StatusNotFound, fmt.Sprintf("name %s already in use", objName)
			}
			objUUID = v
		}
	}

	if objUUID == "" {
		if objName == "" {
			return http.StatusNotFound, fmt.Sprintf("Either a name or UUID must be specified")
		}
		objUUID = uuid.NewV4().String()
		tenantObj.UUID = objUUID
	}

	update := false
	key := path.Join(api.Objs["tenant"].URL, objUUID)
	if err := kvStore.Get(context.Background(), key, &oldtenant); err == nil {
		tenantObj.Status = oldtenant.Status
		tenantObj.UUID = oldtenant.UUID
		update = true
	}

	if err := PreCreateCallback(&tenantObj, update, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}
	log.Infof("Create key: %s object %+v", key, tenantObj)

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Tenant %q creation would be successful", tenantObj.Name)
	}

	if update {
		if err := kvStore.Update(context.Background(), key, &tenantObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	} else {
		if err := kvStore.Create(context.Background(), key, &tenantObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	}
	saveNameUUID(tenantObj.Kind, objName, objUUID)

	return http.StatusOK, "{}"
}

// TenantTestDeleteHandler is
func TenantTestDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return TenantDeleteHandler(w, params, true)
}

// TenantActualDeleteHandler is
func TenantActualDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return TenantDeleteHandler(w, params, false)
}

// TenantDeleteHandler is
func TenantDeleteHandler(w http.ResponseWriter, params martini.Params, dryRun bool) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("tenant", objName)
		if err != nil {
			return http.StatusNotFound, fmt.Sprintf("object %s not found", objName)
		}
		objUUID = v
	}

	key := path.Join(api.Objs["tenant"].URL, objUUID)
	tenant := network.Tenant{}
	if err := kvStore.Get(context.Background(), key, &tenant); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Tenant %q deletion failed: %v\n", objName, err)
	}

	if err := PreDeleteCallback(&tenant, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	if dryRun {
		return http.StatusOK, fmt.Sprintf("Tenant %q creation would be successful", tenant.Name)
	}

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("Tenant %q deletion failed: %v\n", objName, err)
	}
	clearNameUUID("tenant", objName, objUUID)

	out, err := json.Marshal(&tenant)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// TenantGetHandler looks up a tenant.
func TenantGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("tenant", objName)
		if err != nil {
			return http.StatusNotFound, ""
		}
		objUUID = v
	}

	key := path.Join(api.Objs["tenant"].URL, objUUID)

	tenant := network.Tenant{}

	if err := kvStore.Get(context.Background(), key, &tenant); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Tenant %q not found\n", objName)
		}
		return http.StatusInternalServerError, fmt.Sprintf("Tenant %q get failed with error: %v\n", objName, err)
	}

	out, err := json.Marshal(&tenant)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// TenantListHandler lists all tenants.
func TenantListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	tenants := network.TenantList{}

	// FIXME: URL tenant messup bug
	url := api.Objs["tenant"].URL
	url = strings.Replace(url, "//", "/", -1)
	if err := kvStore.List(context.Background(), url, &tenants); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("Tenants not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("Tenants list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&tenants)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}

	return http.StatusOK, string(out)
}

// TenantsWatchHandler establishes a watch on tenants hierarchy.
func TenantsWatchHandler(w http.ResponseWriter, req *http.Request) {
	notifier, ok := w.(http.CloseNotifier)
	if !ok {
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		return
	}
	encoder := json.NewEncoder(w)
	watcher, err := kvStore.PrefixWatch(context.Background(), api.Objs["tenant"].URL, "0")
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

// UserCreateHandler creates a user.
func UserCreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	userObj := api.User{}
	olduser := api.User{}
	if err := decoder.Decode(&userObj); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	dryRun := false
	if strings.HasPrefix(req.URL.Path, "/test") {
		req.URL.Path = strings.TrimPrefix(req.URL.Path, "/test")
		dryRun = true
	}

	objName := userObj.Name
	objUUID := userObj.UUID
	if objName != "" {
		v, err := findUUIDByName("user", objName)
		if err == nil {
			if objUUID != "" && v != objUUID {
				log.Infof("name '%s' is already used by uuid '%s'\n", v)
				return http.StatusNotFound, fmt.Sprintf("name %s already in use", objName)
			}
			objUUID = v
		}
	}

	if objUUID == "" {
		if objName == "" {
			return http.StatusNotFound, fmt.Sprintf("Either a name or UUID must be specified")
		}
		objUUID = uuid.NewV4().String()
		userObj.UUID = objUUID
	}

	update := false
	key := path.Join(api.Objs["user"].URL, objUUID)
	if err := kvStore.Get(context.Background(), key, &olduser); err == nil {
		userObj.Status = olduser.Status
		userObj.UUID = olduser.UUID
		update = true
	}

	if err := PreCreateCallback(&userObj, update, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}
	log.Infof("Create key: %s object %+v", key, userObj)

	if dryRun {
		return http.StatusOK, fmt.Sprintf("User %q creation would be successful", userObj.Name)
	}

	if update {
		if err := kvStore.Update(context.Background(), key, &userObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	} else {
		if err := kvStore.Create(context.Background(), key, &userObj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	}
	saveNameUUID(userObj.Kind, objName, objUUID)

	return http.StatusOK, "{}"
}

// UserTestDeleteHandler is
func UserTestDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return UserDeleteHandler(w, params, true)
}

// UserActualDeleteHandler is
func UserActualDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return UserDeleteHandler(w, params, false)
}

// UserDeleteHandler is
func UserDeleteHandler(w http.ResponseWriter, params martini.Params, dryRun bool) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("user", objName)
		if err != nil {
			return http.StatusNotFound, fmt.Sprintf("object %s not found", objName)
		}
		objUUID = v
	}

	key := path.Join(api.Objs["user"].URL, objUUID)
	user := api.User{}
	if err := kvStore.Get(context.Background(), key, &user); err != nil {
		return http.StatusNotFound, fmt.Sprintf("User %q deletion failed: %v\n", objName, err)
	}

	if err := PreDeleteCallback(&user, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	if dryRun {
		return http.StatusOK, fmt.Sprintf("User %q creation would be successful", user.Name)
	}

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("User %q deletion failed: %v\n", objName, err)
	}
	clearNameUUID("user", objName, objUUID)

	out, err := json.Marshal(&user)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// UserGetHandler looks up a user.
func UserGetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("user", objName)
		if err != nil {
			return http.StatusNotFound, ""
		}
		objUUID = v
	}

	key := path.Join(api.Objs["user"].URL, objUUID)

	user := api.User{}

	if err := kvStore.Get(context.Background(), key, &user); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("User %q not found\n", objName)
		}
		return http.StatusInternalServerError, fmt.Sprintf("User %q get failed with error: %v\n", objName, err)
	}

	out, err := json.Marshal(&user)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// UserListHandler lists all users.
func UserListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	users := api.UserList{}

	// FIXME: URL tenant messup bug
	url := api.Objs["user"].URL
	url = strings.Replace(url, "//", "/", -1)
	if err := kvStore.List(context.Background(), url, &users); err != nil {
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
	watcher, err := kvStore.PrefixWatch(context.Background(), api.Objs["user"].URL, "0")
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
