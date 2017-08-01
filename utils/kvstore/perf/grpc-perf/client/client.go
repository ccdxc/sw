package client

import (
	"context"
	"fmt"
	"os"
	"sync/atomic"
	"time"

	log "github.com/Sirupsen/logrus"

	"github.com/pensando/sw/utils/kvstore/perf/api"
	"github.com/pensando/sw/utils/rpckit"
)

// newClient returns a client
func newClient(url, certFile, keyFile, caFile string) api.RulesClient {
	// disable logging middleware
	rpckit.SetGlobalMiddlewares([]rpckit.Middleware{})

	// create an RPC client
	rpcClient, err := rpckit.NewRPCClient("kv-perf", url, certFile, keyFile, caFile)
	if err != nil {
		log.Errorf("Error connecting to server. Err: %v", err)
		return nil
	}

	return api.NewRulesClient(rpcClient.ClientConn)
}

// RunClient starts the test based on supplied parameters.
func RunClient(gRPCServerURL string, numWatchers, numClients, numPuts, putRate int) {
	log.Infof("Starting test..")
	log.Infof("Num watchers: %v", numWatchers)
	log.Infof("Num putters: %v", numClients)
	log.Infof("Num puts: %v", numPuts)
	log.Infof("Put rate: %v", putRate)

	totalEvents := int32(numPuts * numWatchers)
	numEvents := int32(0)
	watchDoneCh := make(chan struct{})
	putCh := make(chan *api.Rule, 1000)
	putDoneCh := make(chan struct{})
	watchCh := make(chan struct{}, numWatchers)

	dClient := newClient(gRPCServerURL, "", "", "")
	ii := 0
	log.Infof("Waiting for gRPC server to be up ..")
	for {
		if dClient != nil {
			break
		}
		ii++
		if ii > 10 {
			log.Fatalf("gRPC server failed to come up in 10 seconds")
		}
		time.Sleep(time.Second)
		dClient = newClient(gRPCServerURL, "", "", "")
	}
	if _, err := dClient.DeleteRules(context.Background(), &api.DeleteReq{}); err != nil {
		log.Errorf("Failed to delete previous rules, error: %v", err)
	}

	log.Infof("Setting up watchers..")

	for ii := 0; ii < numWatchers; ii++ {
		go func(ii int) {
			rc := newClient(gRPCServerURL, "", "", "")
			if rc == nil {
				log.Fatalf("Watcher %v: failed to create client", ii)
			}
			wc, err := rc.WatchRules(context.Background(), &api.WatchReq{})
			if err != nil {
				log.Fatalf("Watcher %v: failed to watch rules with error: %v", ii, err)
			}
			watchCh <- struct{}{}
			for {
				_, err := wc.Recv()
				if err != nil {
					log.Errorf("Watcher %v: failed to recv event with error: %v", ii, err)
					return
				}
				atomic.AddInt32(&numEvents, 1)
				if atomic.LoadInt32(&numEvents) == int32(totalEvents) {
					watchDoneCh <- struct{}{}
					break
				}
			}
		}(ii)
		time.Sleep(time.Millisecond * 10)
	}

	foundWatchers := 0
	for {
		select {
		case <-watchCh:
			foundWatchers++
		}
		if foundWatchers == numWatchers {
			break
		}
	}

	log.Infof("Setting up put clients..")

	clients := make([]api.RulesClient, 0)
	for ii := 0; ii < numClients; ii++ {
		rc := newClient(":9002", "", "", "")
		clients = append(clients, rc)
	}

	log.Infof("Starting puts..")

	before := time.Now()
	for ii := 0; ii < numPuts; ii++ {
		last := false
		if ii == numPuts-1 {
			last = true
		}
		go func(client api.RulesClient, last bool) {
			rule := <-putCh
			ctx, cancel := context.WithTimeout(context.Background(), time.Second*10)
			if _, err := client.CreateRule(ctx, rule); err != nil {
				log.Fatalf("Failed creating rule %v, error %v", rule.Name, err)
			}
			cancel()
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
			if putRate != 0 {
				time.Sleep(time.Second / time.Duration(putRate))
			}
		}
	}()

	for {
		select {
		case <-putDoneCh:
			log.Infof("Put time: %v", time.Since(before))
		case <-watchDoneCh:
			log.Infof("Total time: %v", time.Since(before))
			log.Infof("Total events received: %v", totalEvents)
			os.Exit(0)
		case <-time.After(time.Second * 10):
			// TODO: Tune this based on number of events expected.
			log.Infof("Got %v events in 10 secs", numEvents)
			os.Exit(1)
		}
	}
}
