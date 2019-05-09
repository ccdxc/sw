// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package main

import (
	"context"
	"flag"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"fmt"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/vos/pkg"
)

var pkgName = globals.Vos

func main() {

	var (
		nsURLs = flag.String("resolver-urls", ":"+globals.CMDResolverPort,
			"comma separated list of resolver URLs of the form 'ip:port'")
		clusterNodes    = flag.String("cluster-nodes", "", "comma seperated list of cluster nodes")
		logFile         = flag.String("logfile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Vos)), "redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		debugFlag       = flag.Bool("debug", false, "enable debug mode")
	)

	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      pkgName,
		Format:      log.JSONFmt,
		Filter:      log.AllowAllFilter,
		Debug:       *debugFlag,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logFile,
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	log.Infof("resolver-urls %+v", nsURLs)
	log.Infof("cluster-nodes %v", *clusterNodes)
	log.Infof("starting object store with args : {%+v}", os.Args)
	hostname := os.Getenv("HOSTNAME")
	nodes := strings.Split(*clusterNodes, ",")
	sort.Strings(nodes)
	found := false
	endpoints := []string{}
	for _, h := range nodes {
		if h == hostname {
			found = true
		}
		endpoints = append(endpoints, fmt.Sprintf("https://%s:%s/disk1", h, globals.VosMinioPort))
		endpoints = append(endpoints, fmt.Sprintf("https://%s:%s/disk2", h, globals.VosMinioPort))
	}
	if len(nodes) > 0 && !found {
		log.Errorf("This node [%v] not included in list of cluster nodes %v, exiting", hostname, nodes)
		return
	}

	args := []string{}
	if len(nodes) > 1 {
		args = []string{pkgName, "server", "--address", fmt.Sprintf(":%s", globals.VosMinioPort)}
		args = append(args, endpoints...)
	} else {
		args = []string{pkgName, "server", "--address", fmt.Sprintf(":%s", globals.VosMinioPort), "/disk1"}
	}
	log.Infof("args for starting Minio %v", args)
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	// init obj store
	err := vospkg.New(ctx, args)
	if err != nil {
		// let the scheduler restart obj store
		log.Fatalf("failed to init object store, %s", err)
	}

	select {}
}
