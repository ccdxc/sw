package main

import (
	"flag"
	"net"
	"net/http"
	"strings"

	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api/cache"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apiserver"
	apisrv "github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	trace "github.com/pensando/sw/venice/utils/trace"
)

const srvName = "ApiServer"

func main() {
	var (
		grpcaddr        = flag.String("grpc-server-port", ":"+globals.APIServerPort, "GRPC Port to listen on")
		kvstore         = flag.String("kvdest", "localhost:2379", "Comma separated list of etcd servers")
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		version         = flag.String("version", "v1", "Version string for native version")
		logToStdoutFlag = flag.Bool("logtostdout", true, "enable logging to stdout")
		logToFile       = flag.String("logtofile", "/var/log/pensando/apiserver.log", "redirect logs to file")
		poolsize        = flag.Int("kvpoolsize", apisrv.DefaultKvPoolSize, "size of KV Store connection pool")
		devmode         = flag.Bool("devmode", true, "Development mode where tracing options are enabled")
		usecache        = flag.Bool("use-cache", true, "Use cache between API server and KV Store")
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
			Format:      log.JSONFmt,
			Filter:      log.AllowAllFilter,
			Debug:       *debugflag,
			CtxSelector: log.ContextAll,
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
		config.DevMode = *devmode
		config.Logger = pl
		config.Version = *version
		config.Scheme = runtime.NewScheme()
		if !*usecache {
			config.Kvstore = store.Config{
				Type:    store.KVStoreTypeEtcd,
				Servers: strings.Split(*kvstore, ","),
				Codec:   runtime.NewJSONCodec(config.Scheme),
			}
		} else {
			var err error
			cachecfg := cache.Config{
				Config: store.Config{
					Type:    store.KVStoreTypeEtcd,
					Servers: strings.Split(*kvstore, ","),
					Codec:   runtime.NewJSONCodec(config.Scheme),
				},
				NumKvClients: *poolsize,
				Logger:       pl,
			}
			config.CacheStore, err = cache.CreateNewCache(cachecfg)
			if err != nil {
				panic("failed to create cache")
			}
		}

		config.KVPoolSize = *poolsize
	}
	trace.Init("ApiServer")
	if *devmode {
		trace.DisableOpenTrace()
	}
	grpclog.SetLogger(pl)
	pl.InfoLog("msg", "Starting Run", "KVStore", config.Kvstore, "GRPCServer", config.GrpcServerPort, "version", config.Version)
	dbgPort := ":" + globals.APIServerRESTPort
	dbgsock, err := net.Listen("tcp", dbgPort)
	if err != nil {
		panic("failed to open debug port")
	}
	go http.Serve(dbgsock, nil)

	srv := apisrvpkg.MustGetAPIServer()
	srv.Run(config)
}
