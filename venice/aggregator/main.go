package main

import (
	"flag"
	"net"
	"net/http"
	"os"
	"strings"
	"time"

	"github.com/influxdata/influxdb/services/httpd"
	"github.com/influxdata/influxdb/services/meta"

	"github.com/pensando/sw/venice/aggregator/gatherer"
	"github.com/pensando/sw/venice/aggregator/rpcserver"
	"github.com/pensando/sw/venice/aggregator/server"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

var (
	listenURL = flag.String("listen-url", ":"+globals.AggregatorAPIPort, "grpc listen address")
	metaDir   = flag.String("meta-dir", "/tmp/meta", "meta dir")
	resolvers = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "Comma separated list of resolver URLs of the form 'ip:port'")
	httpURL   = flag.String("http-url", "", "http listen address")
)

func main() {
	flag.Parse()
	if err := os.MkdirAll(*metaDir, 0777); err != nil {
		log.Fatalf("mkdir all: %s", err)
	}

	mc := meta.NewClient(&meta.Config{Dir: *metaDir, RetentionAutoCreate: true})
	if err := mc.Open(); err != nil {
		log.Fatalf("mc.Open : %s", err)
	}

	_, err := mc.CreateDatabase("NaplesStats")
	if err != nil {
		log.Fatal(err.Error())
	}

	// set influx backends
	setBackends(*resolvers)

	// sync meta
	gatherer.StartBGSync()

	if *httpURL != "" {
		go runHTTP(mc)
	}

	// setup an rpc server
	srv, err := rpcserver.NewAggRPCSrv(*listenURL, mc)
	if err != nil {
		log.Fatalf(err.Error())
	}

	log.Infof("%s is running {%+v}", globals.Aggregator, srv)
	<-srv.ErrOut() // exit when the server does
	os.Exit(-1)
}

func setBackends(resolvers string) {
	rList := strings.Split(resolvers, ",")
	cfg := &resolver.Config{
		Name:    globals.Aggregator,
		Servers: rList,
	}

	rc := resolver.New(cfg)
	for {
		icList := rc.GetURLs(globals.Influx)
		if len(icList) > 0 {
			gatherer.SetBackends(icList)
			return
		}

		time.Sleep(5 * time.Second)
		log.Infof("Aggregator: No Influx service found. Retrying...")
	}

}

func runHTTP(mc *meta.Client) {
	config := httpd.Config{BindAddress: *httpURL, Enabled: true}
	handler := server.NewHandler(config, mc)
	listener, err := net.Listen("tcp", *httpURL)
	if err != nil {
		log.Fatal(err.Error())
	}

	log.Printf("Listening on %s", *httpURL)
	err = http.Serve(listener, handler)
	log.Fatal(err.Error())
}
