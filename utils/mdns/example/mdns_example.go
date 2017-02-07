package main

import (
	"context"
	"flag"
	"fmt"
	"time"

	"github.com/pensando/sw/utils/mdns"
	"github.com/pensando/sw/utils/sysif"
)

const (
	service = "_pensandoCMD._tcp"
	domain  = "local."
	port    = 5555
)

var (
	waitTime = flag.Int("wait", 10, "Duration in [s] to run discovery.")
)

func main() {
	flag.Parse()

	// Get the data to publish
	ni, _ := sysif.GetNodeInfo()
	// Convert the data to map of strings
	data := sysif.MarshallToStringMap(ni)
	// Start a server publishing this data
	s, _ := mdns.NewServer(service, domain, port, data)

	// Context for client
	ctx, cancel := context.WithCancel(context.Background())

	// can pass channel if we are interested in delta events
	c := make(chan *mdns.DiscoveredNodeEvent)
	// Start a client passing context and channel
	b, _ := mdns.NewBrowser(ctx, service, domain, c)

	// When I see events in the channel.
	go func(results <-chan *mdns.DiscoveredNodeEvent) {
		for e := range results {
			fmt.Printf("In Channel  %+v\n", e)
		}
	}(c)

	time.Sleep(time.Second * time.Duration(*waitTime))

	// can ask for complete data available in the cache
	n, _ := b.List()

	// print the obtained data
	for _, v := range n.Items {
		node := sysif.UnmarshallFromStringMap(v.Data)
		fmt.Printf("%s: %+v\n", v.Name, node)
	}

	cancel() // stop the client by cancelling context
	s.Stop() // stop the server and publish to network
}
