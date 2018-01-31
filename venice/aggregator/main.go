package main

import (
	"flag"
	"log"
	"net"
	"net/http"
	"os"

	"github.com/influxdata/influxdb/services/httpd"
	"github.com/influxdata/influxdb/services/meta"

	"github.com/pensando/sw/venice/aggregator/gatherer"
	"github.com/pensando/sw/venice/aggregator/server"
)

var listenURL = flag.String("listen-url", ":8086", "listen address")
var metaDir = flag.String("meta-dir", "/tmp/meta", "meta dir")

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

	// sync meta
	gatherer.StartBGSync()

	config := httpd.Config{BindAddress: *listenURL, Enabled: true}
	handler := server.NewHandler(config, mc)
	listener, err := net.Listen("tcp", *listenURL)
	if err != nil {
		log.Fatal(err.Error())
	}

	log.Printf("Listening on %s", *listenURL)
	err = http.Serve(listener, handler)
	log.Fatal(err.Error())
}
