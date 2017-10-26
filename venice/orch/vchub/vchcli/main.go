/*
 *
 * Copyright 2017, Pensando Systems
 * All rights reserved.
 *
 */

package main

import (
	"flag"
	"fmt"
	"os"

	"github.com/davecgh/go-spew/spew"
	"golang.org/x/net/context"
	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/orch"
)

var (
	listSnics = flag.Bool("list-snics", false, "List SmartNICs")
	listNwifs = flag.Bool("list-nwifs", false, "List NW IFs")
	vchURL    = flag.String("vchub-url", ":"+globals.VCHubAPIPort, "vchub rpc url")
)

func main() {
	flag.Parse()

	if !*listSnics && !*listNwifs {
		fmt.Printf("Need to specify -list-snics or -list-nwifs\n")
		os.Exit(-1)
	}

	// setup client
	var opts []grpc.DialOption
	opts = append(opts, grpc.WithInsecure())
	conn, err := grpc.Dial(*vchURL, opts...)
	if err != nil {
		fmt.Printf("fail to dial: %v", err)
		os.Exit(-1)
	}
	defer conn.Close()
	vcHubClient := orch.NewOrchApiClient(conn)
	if *listSnics {
		listSNICs(vcHubClient)
	}

	if *listNwifs {
		listNwIFs(vcHubClient)
	}

}

func listSNICs(c orch.OrchApiClient) {
	filter := &orch.Filter{}
	nicList, err := c.ListSmartNICs(context.Background(), filter)
	if err != nil {
		fmt.Printf("Error listing nics %v", err)
		return
	}
	nics := nicList.GetItems()
	if len(nics) == 0 {
		fmt.Print("No SNICs found\n")
		return
	}

	for ix, n := range nics {
		fmt.Print("\n======================================\n")
		fmt.Printf("SmartNIC #%d\n", ix)
		spew.Dump(n)
	}
}

func listNwIFs(c orch.OrchApiClient) {
	filter := &orch.Filter{}
	nwifList, err := c.ListNwIFs(context.Background(), filter)
	if err != nil {
		fmt.Printf("Error listing nwifs %v", err)
		return
	}
	nwifs := nwifList.GetItems()
	if len(nwifs) == 0 {
		fmt.Print("No NwIFs found\n")
		return
	}

	for ix, n := range nwifs {
		fmt.Print("\n======================================\n")
		fmt.Printf("NwIF #%d\n", ix)
		spew.Dump(n)
	}
}
