package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"strings"
	"sync/atomic"
	"time"

	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/perf/api"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	prefix     = "/rules/"
	maxRetries = 3
)

func main() {
	var kvServers string
	var numWatchers, numPuts, putRate int
	flag.StringVar(&kvServers, "kv-servers", "http://localhost:2379", "comma separated URLs for kvstore servers")
	flag.IntVar(&numWatchers, "num-watchers", 100, "number of watchers")
	flag.IntVar(&numPuts, "num-puts", 1000, "number of puts")
	flag.IntVar(&putRate, "put-rate", 1000, "number of puts per second")

	flag.Parse()
	log.Infof("Starting test..")
	log.Infof("Num watchers: %v", numWatchers)
	log.Infof("Num puts: %v", numPuts)
	log.Infof("Put rate: %v", putRate)

	// TODO: TLS

	s := runtime.NewScheme()
	s.AddKnownTypes(&api.Rule{}, &api.RuleList{})

	config := store.Config{
		Type:    store.KVStoreTypeEtcd,
		Servers: strings.Split(kvServers, ","),
		Codec:   runtime.NewProtoCodec(s),
	}

	kv, err := store.New(config)
	if err != nil {
		log.Fatalf("Failed to create store with error: %v", err)
	}
	kv.PrefixDelete(context.Background(), prefix)

	totalEvents := int32(numPuts * numWatchers)
	numEvents := int32(0)
	watchDoneCh := make(chan struct{})

	beforeWatchers := time.Now()
	// Create numWatchers number of watchers.
	watchers := make([]kvstore.Watcher, 0)
	for ii := 0; ii < numWatchers; ii++ {
		kv, err := store.New(config)
		if err != nil {
			log.Fatalf("Failed to create watch client %v with error: %v", ii, err)
		}
		watcher, err := kv.PrefixWatch(context.Background(), prefix, "0")
		if err != nil {
			log.Fatalf("Failed to create prefix watch with error: %v", err)
		}
		watchers = append(watchers, watcher)
		go doWatch(ii, watcher, watchDoneCh, &numEvents, totalEvents)
	}
	log.Infof("Time to setup watch clients: %v", time.Since(beforeWatchers))

	afterWatchers := time.Now()
	clients := make([]kvstore.Interface, 0)
	for ii := 0; ii < numWatchers; ii++ {
		kv, err := store.New(config)
		if err != nil {
			log.Fatalf("Failed to create put client %v with error: %v", ii, err)
		}
		clients = append(clients, kv)
	}
	log.Infof("Time to setup put clients: %v", time.Since(afterWatchers))

	putCh := make(chan *api.Rule)
	putDoneCh := make(chan struct{})

	before := time.Now()

	for ii := 0; ii < numPuts; ii++ {
		last := false
		if ii == numPuts-1 {
			last = true
		}
		go func(client kvstore.Interface, last bool) {
			rule := <-putCh
			var err error
			for ii := 0; ii < maxRetries; ii++ {
				if err = client.Create(context.Background(), prefix+rule.Name, rule); err == nil {
					break
				}
				log.Infof("Failed creating rule %v, error %v, retrying...", rule.Name, err)
			}
			if err != nil {
				log.Fatalf("Failed creating rule %v, error %v", rule.Name, err)
			}
			if last {
				putDoneCh <- struct{}{}
			}
		}(clients[ii%len(clients)], last)
	}

	// Add requested number of rules
	go func() {
		for ii := 0; ii < numPuts; ii++ {
			rule := &api.Rule{
				Typemeta: &api.TypeMeta{
					Kind: "Rule",
				},
				Metadata: &api.ObjectMeta{
					Name: fmt.Sprintf("rule-%d", ii),
				},
				Name:     fmt.Sprintf("rule-%d", ii),
				SrcPort:  int32(ii & 0xffff),
				DstPort:  int32(ii & 0xffff),
				SrcIp:    fmt.Sprintf("10.10.10.%d", ii&0xff),
				DstIp:    fmt.Sprintf("20.20.20.%d", ii&0xff),
				Protocol: 6,
				Allow:    true,
			}
			putCh <- rule
			time.Sleep(time.Second / time.Duration(putRate))
		}
	}()

	for {
		select {
		case <-putDoneCh:
			log.Infof("Put time: %v", time.Since(before))
		case <-watchDoneCh:
			log.Infof("Total time: %v", time.Since(before))

			for ii := 0; ii < numWatchers; ii++ {
				watchers[ii].Stop()
			}

			os.Exit(0)
		}
	}
}

func doWatch(id int, watcher kvstore.Watcher, doneCh chan struct{}, numEvents *int32, totalEvents int32) {
	for range watcher.EventChan() {
		atomic.AddInt32(numEvents, 1)
		if atomic.LoadInt32(numEvents) == int32(totalEvents) {
			doneCh <- struct{}{}
			break
		}
	}
}
