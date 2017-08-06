package main

import (
	"context"
	"flag"
	"strings"

	"github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/runtime"
)

const (
	electionName = "master"
	ttl          = 5
)

func main() {
	var etcdServers, id string
	flag.StringVar(&etcdServers, "etcd-servers", "http://localhost:2379", "comma seperated URLs for etcd servers")
	flag.StringVar(&id, "id", "", "identifier to be used in leader election")
	flag.Parse()

	if id == "" {
		log.Fatalf("Please provide the identifier for leader election with --id option")
	}

	config := store.Config{
		Type:    store.KVStoreTypeEtcd,
		Servers: strings.Split(etcdServers, ","),
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
	}

	kv, err := store.New(config)
	if err != nil {
		log.Fatalf("Failed to create store with error: %v", err)
	}

	election, err := kv.Contest(context.Background(), electionName, id, ttl)
	if err != nil {
		log.Fatalf("Failed to start the leader election with error: %v", err)
	}

	for {
		select {
		case e := <-election.EventChan():
			log.Infof("Election event: %+v", e)
		}
	}
}
