package rest

import (
	"context"
	"encoding/json"
	"net/http"

	log "github.com/Sirupsen/logrus"
	"github.com/go-martini/martini"

	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/cmd/ops"
	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/utils/errors"
	"github.com/pensando/sw/utils/kvstore"
)

const (
	uRLPrefix  = "/api/v1"
	clusterURL = "/cluster"
	nodesURL   = "/nodes"
)

// NewRESTServer creates REST server endpoints for cluster create/get. These ops
// are handled directly by CMD before the cluster is created. Once the cluster
// is created, all ops come through API Gateway over gRPC.
func NewRESTServer() *martini.ClassicMartini {
	m := martini.Classic()

	m.Post(uRLPrefix+clusterURL, ClusterCreateHandler)
	m.Get(uRLPrefix+clusterURL, ClusterGetHandler)

	m.Get(uRLPrefix+nodesURL, NodeListHandler)

	return m
}

// ClusterCreateHandler handles the REST call for cluster creation.
func ClusterCreateHandler(w http.ResponseWriter, req *http.Request) {
	env.Mutex.Lock()
	defer env.Mutex.Unlock()

	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	cluster := cmd.Cluster{}
	if err := decoder.Decode(&cluster); err != nil {
		errors.SendBadRequest(w, err.Error())
		return
	}

	log.Infof("Cluster create args: %+v", cluster)

	ops.RunHTTP(w, ops.NewClusterCreateOp(&cluster))
}

// ClusterGetHandler returns the cluster information.
func ClusterGetHandler(w http.ResponseWriter, req *http.Request) {
	cluster := cmd.Cluster{}

	if env.KVStore == nil {
		errors.SendNotFound(w, "Cluster", "")
		return
	}

	if err := env.KVStore.Get(context.Background(), globals.ClusterKey, &cluster); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			errors.SendNotFound(w, "Cluster", "")
			return
		}
		errors.SendInternalError(w, err)
		return
	}

	cluster.Status.Leader = env.LeaderService.Leader()

	encoder := json.NewEncoder(w)

	if err := encoder.Encode(&cluster); err != nil {
		log.Errorf("Failed to encode with error: %v", err)
	}
}

// NodeListHandler returns the nodes belonging to the cluster.
func NodeListHandler(w http.ResponseWriter, req *http.Request) {
	nodes := cmd.NodeList{}

	if env.KVStore == nil {
		errors.SendNotFound(w, "NodeList", "")
		return
	}

	if err := env.KVStore.List(context.Background(), globals.NodesKey, &nodes); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			errors.SendNotFound(w, "NodeList", "")
			return
		}
		errors.SendInternalError(w, err)
		return
	}

	nodes.Kind = "NodeList"

	encoder := json.NewEncoder(w)

	if err := encoder.Encode(&nodes); err != nil {
		log.Errorf("Failed to encode with error: %v", err)
	}
}
