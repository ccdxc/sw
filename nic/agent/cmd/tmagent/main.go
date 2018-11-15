package main

import (
	"context"
	"flag"
	"fmt"
	"net"
	"path/filepath"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/nodewatcher"

	"github.com/pensando/sw/nic/agent/ipc"
	ipcproto "github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi"
	delphi "github.com/pensando/sw/nic/delphi/gosdk"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/ntsdb"
	"github.com/pensando/sw/venice/utils/resolver"
)

var fwTable ntsdb.Table

type service struct {
	name         string
	sysmgrClient *sysmgr.Client
}

var srv = &service{
	name: "tmagent",
}

func (s *service) OnMountComplete() {
	log.Printf("OnMountComplete() done for %s\n", s.name)
	s.sysmgrClient.InitDone()
}

func (s *service) Name() string {
	return s.name
}

func reportMetrics(nodeUUID string, rc resolver.Interface) error {
	// report node metrics
	node := &cluster.SmartNIC{
		TypeMeta: api.TypeMeta{
			Kind: "SmartNIC",
		},
		ObjectMeta: api.ObjectMeta{
			Name: nodeUUID,
		},
	}

	return nodewatcher.NewNodeWatcher(context.Background(), node, rc, 30, log.WithContext("pkg", "nodewatcher"))
}

func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		primaryMAC      = flag.String("primary-mac", "", "Primary MAC address")
		hostIf          = flag.String("hostif", "ntrunk0", "Host facing interface")
		mode            = flag.String("mode", "classic", "specify the agent mode either classic or managed")
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

	var macAddr net.HardwareAddr

	if *primaryMAC != "" {
		mac, err := net.ParseMAC(*primaryMAC)
		if err != nil {
			log.Fatalf("invalid primary-mac %v", *primaryMAC)
		}
		macAddr = mac
	} else {
		mac, err := netutils.GetIntfMac(*hostIf)
		if err != nil {
			log.Fatalf("Error getting host interface's mac addr. Err: %v", err)
		}
		macAddr = mac
	}

	mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
	instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
	shm, err := ipc.NewSharedMem(mSize, instCount, "/fwlog_ipc_shm")
	if err != nil {
		log.Fatal(err)
	}

	if *mode == "managed" {
		rList := strings.Split(*resolverURLs, ",")
		cfg := &resolver.Config{
			Name:    globals.Tmagent,
			Servers: rList,
		}

		rc := resolver.New(cfg)

		if err := reportMetrics(macAddr.String(), rc); err != nil {
			log.Fatal(err)
		}

		fwTable, err = ntsdb.NewObj("firewall", map[string]string{}, &ntsdb.TableOpts{})
		if err != nil {
			log.Fatal(err)
		}

		for ix := 0; ix < instCount; ix++ {
			ipc := shm.IPCInstance()
			go ipc.Receive(context.Background(), processFWEvent)
		}

	} else { // todo: fwlog export to external syslog server
	}

	_, err = restapi.NewRestServer(*restURL)
	if err != nil {
		log.Errorf("Error creating the rest API server. Err: %v", err)
	}

	delphiClient, err := delphi.NewClient(srv)
	if err != nil {
		log.Fatalf("delphi NewClient failed")
	}
	srv.sysmgrClient = sysmgr.NewClient(delphiClient, srv.Name())

	// run delphi thread in background
	go delphiClient.Run()

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
