package main

import (
	"flag"
	"strings"

	"google.golang.org/grpc/grpclog"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks"
	apisrv "github.com/pensando/sw/apiserver"
	apisrvpkg "github.com/pensando/sw/apiserver/pkg"
	"github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/runtime"
	trace "github.com/pensando/sw/utils/trace"
)

const srvName = "ApiServer"

func main() {
	var (
		grpcaddr        = flag.String("grpc-server-port", ":8082", "GRPC Port to listen on")
		kvstore         = flag.String("kvdest", "localhost:2379", "Comma seperated list of etcd servers")
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		version         = flag.String("version", "v1", "Version string for native version")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		logToFile       = flag.String("logtofile", "/var/log/pensando/apiserver.log", "redirect logs to file")
	)

	flag.Parse()

	var pl log.Logger
	{
		logtoFileFlag := true
		if *logToFile == "" {
			logtoFileFlag = false
		}

		logConfig := &log.Config{
			Module:      srvName,
			Format:      log.LogFmt,
			Filter:      log.AllowAllFilter,
			Debug:       *debugflag,
			Context:     true,
			LogToStdout: *logToStdoutFlag,
			LogToFile:   logtoFileFlag,
			FileCfg: log.FileConfig{
				Filename:   *logToFile,
				MaxSize:    10, // TODO: These needs to be part of Service Config Object
				MaxBackups: 3,  // TODO: These needs to be part of Service Config Object
				MaxAge:     7,  // TODO: These needs to be part of Service Config Object
			},
		}
		pl = log.GetNewLogger(logConfig)
	}

	var config apisrv.Config
	{
		config.GrpcServerPort = *grpcaddr
		config.DebugMode = *debugflag
		config.Logger = pl
		config.Version = *version
		config.Scheme = runtime.NewScheme()
		config.Kvstore = store.Config{
			Type:    store.KVStoreTypeEtcd,
			Servers: strings.Split(*kvstore, ","),
			Codec:   runtime.NewJSONCodec(config.Scheme),
		}
	}
	trace.Init("ApiServer")
	grpclog.SetLogger(pl)
	pl.InfoLog("msg", "Starting Run", "KVStore", config.Kvstore, "GRPCServer", config.GrpcServerPort, "version", config.Version)
	srv := apisrvpkg.MustGetAPIServer()
	srv.Run(config)
}
