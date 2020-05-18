package impl

import (
	"context"

	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/listerwatcher"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type aggWatchHooks struct {
	l   log.Logger
	svc apiserver.Service
}

func registerAggWatchHooks(svc apiserver.Service, logger log.Logger) {
	h := aggWatchHooks{}
	h.svc = svc
	h.l = logger
	// Override the Watcher with a watcher that does allow cross service watch functionality.
	logger.InfoLog("Service", "AggWatchV1", "msg", "registering hooks")
	svc.WithKvWatchFunc(func(l log.Logger, options *api.AggWatchOptions, kvs kvstore.Interface, stream interface{}, txfnMap map[string]func(from, to string, i interface{}) (interface{}, error), version, svcprefix string) error {
		wstream := stream.(grpc.ServerStream)
		nctx, cancel := context.WithCancel(wstream.Context())
		defer cancel()
		watcher, err := kvs.WatchAggregate(nctx, *options)
		if err != nil {
			l.ErrorLog("msg", "error starting Watch for service", "err", err, "service", "{{.Name}}")
			return err
		}
		// add all transform functions needed.
		txnfns := make(map[string]func(from, to string, i interface{}) (interface{}, error))
		svcMap := make(map[string]bool)
		for _, v := range options.WatchOptions {
			svcMap[v.Group] = true
		}

		for k := range svcMap {
			svcs := apisrvpkg.MustGetAPIServer().GetServiceForGroup(k)
			for i := range svcs {
				svcs[i].PopulateTxfmMap(txnfns)
			}
		}
		return listerwatcher.SvcWatch(nctx, watcher, wstream, txnfns, version, l)
	})

}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("aggwatch.AggWatchV1", registerAggWatchHooks)
}
