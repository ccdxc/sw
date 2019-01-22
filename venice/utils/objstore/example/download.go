package main

import (
	"context"
	"crypto/tls"
	"flag"
	"io"
	"os"
	"strings"

	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
)

func main1() {
	bucket := flag.String("bucket", "images", "bucket name")
	name := flag.String("name", "", "object name")
	out := flag.String("output", "", "output file name")
	servers := flag.String("resolvers", "node1:9009", "comma seperated resolvers")
	flag.Parse()

	if *name == "" || *out == "" {
		log.Fatal("filename and output file needed")
	}
	rslvrs := strings.Split(*servers, ",")
	opts := []rpckit.Option{}
	tlsp, err := tlsproviders.NewDefaultCMDBasedProvider("node1:9002", "vostest")
	if err != nil {
		log.Fatalf("error getting tls provider (%s)", err)
	}
	opts = append(opts, rpckit.WithTLSProvider(tlsp))
	resolver := resolver.New(&resolver.Config{Name: "TestClient", Servers: rslvrs, Options: opts})
	tlsc := tls.Config{
		InsecureSkipVerify: true,
	}
	client, err := objstore.NewClient("default", *bucket, resolver, objstore.WithTLSConfig(&tlsc))
	if err != nil {
		log.Fatalf("could not create client (%s)", err)
	}
	ctx := context.Background()
	fr, err := client.GetObject(ctx, *name)
	if err != nil {
		log.Fatalf("could not get object (%s)", err)
	}

	of, err := os.Create(*out)
	if err != nil {
		log.Fatalf("could not create output file [%s](%s)", *out, err)
	}
	defer of.Close()
	buf := make([]byte, 1024)
	totsize := 0
	for {
		n, err := fr.Read(buf)
		if err != nil && err != io.EOF {
			log.Fatalf("error while reading object (%s)", err)
		}
		if n == 0 {
			break
		}
		totsize += n
		if _, err = of.Write(buf[:n]); err != nil {
			log.Fatalf("error writing to output file (%s)", err)
		}
	}
	log.Infof("Got image [%v] of size [%d]", *name, totsize)
}
