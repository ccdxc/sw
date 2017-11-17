package server

import (
	"encoding/json"
	"net/http"
	"path"
	"sync"
	"time"

	"github.com/go-martini/martini"
	"golang.org/x/net/context"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/venice/utils/errors"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/perf/api"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	// URLPrefix is the api url prefix.
	URLPrefix = "/api/v1"
	// RulesURL is the url for rules endpoint.
	RulesURL = "/rules"
)

var (
	mutex      sync.Mutex
	clients    []kvstore.Interface
	numClients int
	clientID   int
)

// NewRESTServer creates a new RESTful server.
func NewRESTServer() *martini.ClassicMartini {
	m := martini.Classic()

	m.Post(URLPrefix+RulesURL, RuleCreateHandler)
	m.Get(URLPrefix+RulesURL, RuleListHandler)

	return m
}

// RuleCreateHandler creates a rule.
func RuleCreateHandler(w http.ResponseWriter, req *http.Request) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	rule := api.Rule{}
	if err := decoder.Decode(&rule); err != nil {
		errors.SendBadRequest(w, err.Error())
		return
	}

	encoder := json.NewEncoder(w)
	mutex.Lock()
	client := clients[clientID%numClients]
	clientID++
	mutex.Unlock()
	if err := client.Create(context.Background(), path.Join("/rules", rule.Name), &rule); err != nil {
		encoder.Encode(err)
	} else {
		encoder.Encode(&rule)
	}
}

// RuleListHandler lists all the configured rules.
func RuleListHandler(w http.ResponseWriter, req *http.Request) {
	rules := api.RuleList{}

	if err := clients[0].List(context.Background(), "/rules", &rules); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			errors.SendNotFound(w, "NodeList", "")
			return
		}
		errors.SendInternalError(w, err)
		return
	}

	encoder := json.NewEncoder(w)
	before := time.Now()
	encoder.Encode(&rules)
	log.Infof("Encode time %v", time.Since(before))
}

// RunServer creates a gRPC server for rule operations.
func RunServer(url string, client kvstore.Interface, stopChannel chan bool) {
	watcher, err := client.PrefixWatch(context.Background(), "/rules", "0")
	if err != nil {
		log.Fatalf("Failed to establish watch, error: %v", err)
	}

	// create an rpc handler object
	h := &rulesRPCHandler{
		watchChs: make([]chan *kvstore.WatchEvent, 0),
		watcher:  watcher,
	}

	// create an RPC server with logging disabled
	rpcServer, err := rpckit.NewRPCServer("rules", url, rpckit.WithLoggerEnabled(false))
	if err != nil {
		log.Fatalf("Error creating grpc server: %v", err)
	}

	go h.handleWatches()

	// register the RPC handler and start the server
	api.RegisterRulesServer(rpcServer.GrpcServer, h)
	rpcServer.Start()
	defer func() { rpcServer.Stop() }()

	// wait forever
	<-stopChannel
}

// rulesRPCHandler handles all rule gRPC calls.
type rulesRPCHandler struct {
	sync.Mutex
	watchChs []chan *kvstore.WatchEvent
	watcher  kvstore.Watcher
}

func (r *rulesRPCHandler) addWatcher(ch chan *kvstore.WatchEvent) {
	r.Lock()
	defer r.Unlock()
	r.watchChs = append(r.watchChs, ch)
}

func (r *rulesRPCHandler) handleWatches() {
	for in := range r.watcher.EventChan() {
		if in.Type == kvstore.Deleted {
			continue
		}
		for _, watchCh := range r.watchChs {
			watchCh <- in
		}
	}
}

// CreateRule creates a rule.
func (r *rulesRPCHandler) CreateRule(ctx context.Context, rule *api.Rule) (*api.Rule, error) {
	r.Lock()
	client := clients[clientID%numClients]
	clientID++
	r.Unlock()
	if err := client.Create(ctx, path.Join("/rules", rule.Name), rule); err != nil {
		return nil, err
	}
	return rule, nil
}

// ListRules lists all the rules.
func (r *rulesRPCHandler) ListRules(ctx context.Context, req *api.ListReq) (*api.RuleList, error) {
	rules := api.RuleList{}
	if err := clients[0].List(ctx, "/rules", &rules); err != nil {
		return nil, err
	}
	return &rules, nil
}

// DeleteRules deletes all the rules.
func (r *rulesRPCHandler) DeleteRules(ctx context.Context, req *api.DeleteReq) (*api.RuleList, error) {
	if err := clients[0].PrefixDelete(ctx, "/rules"); err != nil {
		return nil, err
	}
	return &api.RuleList{}, nil
}

// WatchRules watches changes to rules.
func (r *rulesRPCHandler) WatchRules(req *api.WatchReq, server api.Rules_WatchRulesServer) error {
	ch := make(chan *kvstore.WatchEvent, 0)
	r.addWatcher(ch)
	for in := range ch {
		out := &api.Event{
			Rule: in.Object.(*api.Rule),
		}
		switch in.Type {
		case kvstore.Created:
			out.Type = api.Event_Created
		case kvstore.Updated:
			out.Type = api.Event_Updated
		case kvstore.Deleted:
			out.Type = api.Event_Deleted
		}
		if err := server.Send(out); err != nil {
			log.Errorf("Error sending event %v, %v", in, err)
			return err
		}
	}

	return nil
}

// RunServers runs both REST and gRPC Servers.
func RunServers(restServerURL, gRPCServerURL string, etcdServers []string, numEtcdClients int) {
	s := runtime.NewScheme()
	s.AddKnownTypes(&api.Rule{})

	clients = make([]kvstore.Interface, 0)
	numClients = numEtcdClients

	log.Infof("Using etcd servers: %v", etcdServers)
	for ii := 0; ii < numClients+1; ii++ {
		sConfig := store.Config{
			Type:    store.KVStoreTypeEtcd,
			Servers: etcdServers,
			Codec:   runtime.NewProtoCodec(s),
		}
		kv, err := store.New(sConfig)
		if err != nil {
			log.Fatalf("Failed to create kv store, error: %v", err)
			return
		}
		clients = append(clients, kv)
	}

	go NewRESTServer().RunOnAddr(restServerURL)
	RunServer(gRPCServerURL, clients[numClients], nil)
	log.Fatalf("Server terminated")
}
