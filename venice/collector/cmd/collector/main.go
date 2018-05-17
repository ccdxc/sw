package main

import (
	"flag"
	"os"
	"strings"
	"time"

	"golang.org/x/net/context"

	tec "github.com/pensando/sw/venice/collector"
	"github.com/pensando/sw/venice/collector/rpcserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

type cliOpts struct {
	listenURL       string
	resolverURLs    string
	logToStdoutFlag bool
}

func main() {
	var opts cliOpts

	flagSet := flag.NewFlagSet("collector", flag.ContinueOnError)
	flagSet.StringVar(&opts.listenURL,
		"listen-url",
		":"+globals.CollectorAPIPort,
		"IP:Port where collector api server should listen for grpc")
	flagSet.StringVar(&opts.resolverURLs,
		"resolver-urls",
		":"+globals.CMDResolverPort,
		"Comma separated list of resolver URLs of the form 'ip:port'")
	flagSet.BoolVar(&opts.logToStdoutFlag,
		"logtostdout",
		false,
		"Enable logging to stdout")

	err := flagSet.Parse(os.Args[1:])

	if err != nil {
		log.Errorf("Error %v parsing args", err)
		os.Exit(1)
	}

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "collector",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		LogToStdout: opts.logToStdoutFlag,
		LogToFile:   true,
		CtxSelector: log.ContextAll,
		FileCfg: log.FileConfig{
			Filename:   "/var/log/pensando/collector.log",
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	// Wait until we get a backend
	beList := getBackEnds(opts.resolverURLs)

	// create a collector
	c := tec.NewCollector(context.Background())
	for _, be := range beList {
		c.AddBackEnd("http://" + be)
		log.Infof("Added Backend %s", be)
	}

	// setup an rpc server
	srv, err := rpcserver.NewCollRPCSrv(opts.listenURL, c)
	if err != nil {
		log.Fatalf(err.Error())
	}

	<-srv.ErrOut() // exit when the server does
	os.Exit(-1)
}

// getBackEnds fetches the influx backends from resolver
func getBackEnds(resolvers string) []string {
	rList := strings.Split(resolvers, ",")
	cfg := &resolver.Config{
		Name:    globals.Collector,
		Servers: rList,
	}

	rc := resolver.New(cfg)
	for {
		icList := rc.GetURLs(globals.Influx)
		if len(icList) > 0 {
			return icList
		}

		time.Sleep(5 * time.Second)
		log.Infof("No Influx service found. Retrying...")
	}
}
