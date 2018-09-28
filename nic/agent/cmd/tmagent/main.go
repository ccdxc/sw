package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"
	"strings"
	"time"

	"github.com/pensando/sw/nic/agent/ipc"
	ipcproto "github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/ntsdb"
	"github.com/pensando/sw/venice/utils/resolver"
)

var fwTable ntsdb.Table

func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Tmagent)), "Redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:Port>")
		restURL         = flag.String("rest-url", ":"+globals.TmAGENTRestPort, "specify Agent REST URL")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.Tmagent,
		Format:      log.JSONFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       *debugflag,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
	instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
	shm, err := ipc.NewSharedMem(mSize, instCount, "/fwlog_ipc_shm")
	if err != nil {
		log.Fatal(err)
	}

	rList := strings.Split(*resolverURLs, ",")
	cfg := &resolver.Config{
		Name:    globals.Citadel,
		Servers: rList,
	}

	rc := resolver.New(cfg)
	tsdbOpts := &ntsdb.Opts{ClientName: "tmAgent",
		ResolverClient: rc,
		Collector:      globals.Citadel,
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

	_, err = restapi.NewRestServer(*restURL)
	if err != nil {
		log.Errorf("Error creating the rest API server. Err: %v", err)
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
