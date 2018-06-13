package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"strings"
	"time"

	"github.com/pensando/sw/nic/agent/ipc"
	ipcproto "github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/ntsdb"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var fwTable ntsdb.Table

func main() {
	var resolverURLs string
	flagSet := flag.NewFlagSet("tmagent", flag.ContinueOnError)
	flagSet.StringVar(&resolverURLs,
		"resolver-urls",
		":"+globals.CMDResolverPort,
		"IP:Port of resolver")

	err := flagSet.Parse(os.Args[1:])

	if err != nil {
		log.Errorf("Error %v parsing args", err)
		os.Exit(1)
	}
	// Set the TLS provider for rpckit
	rpckit.SetN4STLSProvider()

	mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
	instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
	shm, err := ipc.NewSharedMem(mSize, instCount, "/fwlog_ipc_shm")
	if err != nil {
		log.Fatal(err)
	}

	rList := strings.Split(resolverURLs, ",")
	cfg := &resolver.Config{
		Name:    globals.Collector,
		Servers: rList,
	}

	rc := resolver.New(cfg)
	tsdbOpts := &ntsdb.Opts{ClientName: "tmAgent",
		ResolverClient: rc,
		Collector:      globals.Collector,
		DBName:         "FWLogs",
	}
	ntsdb.Init(context.Background(), tsdbOpts)

	fwTable, err = ntsdb.NewTable("firewall", &ntsdb.TableOpts{})
	if err != nil {
		log.Fatal(err)
	}

	for ix := 0; ix < instCount; ix++ {
		ipc := shm.IPCInstance()
		go ipc.Receive(context.Background(), processFWEvent)
	}

	// wait forever
	select {}
}

func processFWEvent(ev *ipcproto.FWEvent, ts time.Time) {
	ipSrc := netutils.IPv4Uint32ToString(ev.GetSipv4())
	ipDest := netutils.IPv4Uint32ToString(ev.GetDipv4())
	dPort := fmt.Sprintf("%v", ev.GetDport())
	ipProt := fmt.Sprintf("%v", ev.GetIpProt())
	action := fmt.Sprintf("%v", ev.GetFlowaction())
	dir := fmt.Sprintf("%v", ev.GetDirection())
	unixnano := ev.GetTimestamp()
	if unixnano != 0 {
		// if a timestamp was specified in the msg, use it
		ts = time.Unix(0, unixnano)
	}
	fwTable.Point(map[string]string{"src": ipSrc, "dest": ipDest, "dPort": dPort, "ipProt": ipProt, "action": action, "direction": dir},
		map[string]interface{}{"sPort": int64(ev.GetSport())}, ts)
}
