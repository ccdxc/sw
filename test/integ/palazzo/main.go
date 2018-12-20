// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"flag"
	"fmt"
	"os"
	"os/signal"
	"syscall"
	"time"

	check "gopkg.in/check.v1"

	_ "github.com/influxdata/influxdb/tsdb/engine"
	_ "github.com/influxdata/influxdb/tsdb/index"

	veniceinteg "github.com/pensando/sw/test/integ/venice_integ"
	"github.com/pensando/sw/venice/utils/log"
)

func main() {
	// Fill logger config params
	logConfig := &log.Config{
		Module:      "palazzo",
		Format:      log.LogFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	// command line args
	var numHosts = flag.Int("hosts", 3, "Number of hosts / agents")
	var datapathKind = flag.String("datapath", "mock", "Specify the datapath type. mock | hal | delphi")
	var skipAuth = flag.Bool("skipauth", false, "Skip Authorization")
	var apigwPort = flag.String("apiport", "", "API Gateway port (default 9090)")
	var startElastic = flag.Bool("elastic", true, "Start Elasticsearch")
	flag.Parse()

	config := veniceinteg.GetDefaultSuiteConfig()
	config.NumHosts = *numHosts
	config.DatapathKind = *datapathKind
	config.APIGatewaySkipAuth = *skipAuth
	config.StartEventsAndSearch = *startElastic
	config.ElasticSearchName = "palazzo-es"
	if *apigwPort != "" {
		config.APIGatewayPort = *apigwPort
	}

	// change the working directory to serve the UI files
	gopath, isGopathSet := os.LookupEnv("GOPATH")
	if isGopathSet {
		os.Chdir(gopath + "/src/github.com/pensando/sw/tools/docker-files/apigw")
	}

	// create venice integ suite
	veniceInteg := veniceinteg.NewVeniceIntegSuite(config)

	// start all processes
	veniceInteg.SetUpSuite(&check.C{})

	time.Sleep(time.Second)
	fmt.Printf("\n\n\n\n================================ Venice Emulator is ready =====================================\n\n\n\n")

	// wait for signal
	sigs := make(chan os.Signal, 1)
	done := make(chan bool, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)

	// This goroutine executes a blocking receive for  signals.
	go func() {
		sig := <-sigs
		fmt.Println(sig)
		done <- true
	}()

	fmt.Println("awaiting signal")
	<-done
	fmt.Println("================ Venice emulato is stopping =====================")

	veniceInteg.TearDownSuite(&check.C{})
	fmt.Println("================ Venice emulator exited ================")
}
