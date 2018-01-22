package main

import (
	"flag"
	"fmt"
	"net/url"
	"os"
	"strings"
	"time"

	"github.com/vmware/govmomi/vim25/soap"
	"golang.org/x/net/context"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/orch/vchub/defs"
	"github.com/pensando/sw/venice/orch/vchub/server"
	"github.com/pensando/sw/venice/orch/vchub/store"
	"github.com/pensando/sw/venice/orch/vchub/vcprobe"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	storeQSize = 64
)

type cliOpts struct {
	listenURL   string
	storeType   string
	storeURL    string
	vcenterList []*url.URL
}

func printUsage(f *flag.FlagSet) {
	fmt.Fprintf(os.Stderr, "Usage: %s [OPTION]...\n", os.Args[0])
	f.PrintDefaults()
}

func parseOpts(opts *cliOpts) error {
	var storeArg string
	var vcList string
	var resolverURLs string

	flagSet := flag.NewFlagSet("vchub", flag.ContinueOnError)
	flagSet.StringVar(&opts.listenURL,
		"listen-url",
		":"+globals.VCHubAPIPort,
		"IP:Port where vchub api server should listen for grpc")
	flagSet.StringVar(&storeArg,
		"store-url",
		"memkv:",
		"url for vchub's datastore type:url")
	flagSet.StringVar(&vcList,
		"vcenter-list",
		"",
		"Comma separated list of vc URL of the form 'https://user:pass@ip:port'")
	flagSet.StringVar(&resolverURLs,
		"resolver-urls",
		":"+globals.CMDResolverPort,
		"Comma separated list of resolver URLs of the form 'ip:port'")

	err := flagSet.Parse(os.Args[1:])

	if err != nil {
		log.Errorf("Error %v parsing args", err)
		return err
	}

	if vcList == "" {
		printUsage(flagSet)
		log.Errorf("vcenter-list cannot be empty")
		return fmt.Errorf("vcenter-list cannot be empty")
	}

	s := strings.Split(storeArg, ":")
	if s[0] != "etcd" && s[0] != "memkv" {
		printUsage(flagSet)
		log.Errorf("store type must be etcd or memkv")
		return fmt.Errorf("store type must be etcd or memkv")
	}
	opts.storeType = s[0]
	opts.storeURL = s[1]
	vcs := strings.Split(vcList, ",")
	opts.vcenterList = make([]*url.URL, len(vcs))
	for ix, vc := range vcs {
		opts.vcenterList[ix], err = soap.ParseURL(vc)
		if err != nil {
			log.Errorf("Error %v parsing url %s", err, vc)
			return fmt.Errorf("Error %v parsing url %s", err, vc)
		}
	}

	return nil
}

func waitForever() {
	select {}
}

func launchVCHub(opts *cliOpts) {
	// Initialize store and start grpc server
	store.Init(opts.storeURL, opts.storeType)
	_, err := server.NewVCHServer(opts.listenURL)
	if err != nil {
		log.Errorf("VCHServer start failed %v", err)
		os.Exit(1)
	}

	storeCh := make(chan defs.StoreMsg, storeQSize)
	vchStore := store.NewVCHStore(context.Background())
	vchStore.Run(storeCh)

	// Start probes
	retryMap := make(map[string]*vcprobe.VCProbe)
	for _, u := range opts.vcenterList {
		vcp := vcprobe.NewVCProbe(u, storeCh)
		if vcp.Start() == nil {
			vcp.Run()
		} else {
			vcp.Stop()
			retryMap[u.String()] = vcp
		}
	}

	for {
		if len(retryMap) == 0 {
			break
		}

		time.Sleep(time.Second)
		for u, v := range retryMap {
			if v.Start() == nil {
				v.Run()
				delete(retryMap, u)
			} else {
				v.Stop()
			}
		}

	}
}

func main() {
	var opts cliOpts

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "vchub",
		Format:      log.JSONFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		LogToStdout: true,
		LogToFile:   true,
		CtxSelector: log.ContextAll,
		FileCfg: log.FileConfig{
			Filename:   "/tmp/vchub.log",
			MaxSize:    10, // TODO: These needs to be part of Service Config Object
			MaxBackups: 3,  // TODO: These needs to be part of Service Config Object
			MaxAge:     7,  // TODO: These needs to be part of Service Config Object
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)
	err := parseOpts(&opts)
	if err != nil {
		os.Exit(1)
	}

	launchVCHub(&opts)
	waitForever()
}
