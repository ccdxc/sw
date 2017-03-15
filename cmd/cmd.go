// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"context"
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

// Wait in an infinite loop and keep printing all the list of nodes every 10 seconds.
func main() {
	// Get the data to publish
	ni, _ := sysif.GetNodeInfo()
	data := sysif.MarshallToStringMap(ni)
	mdns.NewServer(service, domain, port, data)

	ctx := context.Background()
	b, _ := mdns.NewBrowser(ctx, service, domain, nil)

	for {
		time.Sleep(10 * time.Second)

		// can ask for complete data available in the cache
		n, _ := b.List()

		// print the obtained data
		for _, v := range n.Items {
			node := sysif.UnmarshallFromStringMap(v.Data)
			fmt.Printf("%s: %+v\n", v.Name, node)
		}
	}
}
