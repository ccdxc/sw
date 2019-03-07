package main

import (
	"context"
	"crypto/tls"
	"flag"
	"fmt"
	"io"
	"net/http"
	"os"
	"reflect"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	penobj "github.com/pensando/sw/api/generated/objstore"
	penobjcl "github.com/pensando/sw/api/generated/objstore/grpc/client"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
)

var useReflect bool

func viaGrpc(ctx context.Context, bucket, name string, of *os.File, clnt penobj.ServiceObjstoreV1Client) {
	obj := penobj.Object{
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: bucket,
			Tenant:    "default",
		},
	}

	stream, err := clnt.DownloadFile(ctx, &obj)
	if err != nil {
		log.Fatalf("failed to start stream (%s)", err)
	}
	now := time.Now()
	for {
		resp, err := stream.Recv()
		if err == io.EOF {
			log.Infof("Downloaded file in [%v]", time.Since(now))
			return
		}
		if err != nil {
			log.Fatalf("receive return error(%s)", err)
			return
		}
		var buf []byte
		if useReflect {
			v := reflect.ValueOf(resp)
			if v.Kind() == reflect.Ptr {
				v = reflect.Indirect(v)
			}
			fv := v.FieldByName("Content").Interface()
			buf = fv.([]byte)
		} else {
			buf = resp.Content
		}

		if _, err = of.Write(buf); err != nil {
			log.Fatalf("could not write to output (%s)", err)
			return
		}
	}
}

func viaDirect(dest, name string, tlsConfig *tls.Config, of *os.File) {
	url := dest + "/apis/v1/downloads/images/" + name
	http.DefaultTransport.(*http.Transport).TLSClientConfig = tlsConfig
	now := time.Now()
	resp, err := http.Get(url)
	if err != nil {
		log.Fatalf("failed to do get on direct endpoint (%s)", err)
	}
	cnt, err := io.Copy(of, resp.Body)
	if err != nil {
		log.Fatalf("Failed to write to file (%s)", err)
	}
	log.Infof("downloaded [%d] bytes  to [%v] in [%v]", cnt, of.Name(), time.Since(now))

}

func main1() {
	bucket := flag.String("bucket", "images", "bucket name")
	name := flag.String("name", "", "object name")
	out := flag.String("output", "", "output file name")
	servers := flag.String("resolvers", "node1:9009", "comma seperated resolvers")
	useGrpc := flag.Bool("grpc", false, "User gRPC")
	useDirect := flag.Bool("direct", false, "user direct download")
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
	of, err := os.Create(*out)
	if err != nil {
		log.Fatalf("could not create output file [%s](%s)", *out, err)
	}
	defer of.Close()

	switch {
	case *useGrpc:
		{
			grpcConn, err := rpckit.NewRPCClient("testclient", globals.Vos, rpckit.WithBalancer(balancer.New(resolver)), rpckit.WithTLSProvider(tlsp))
			if err != nil {
				log.Fatalf("Failed to create grpc client (%s)", err)
			}
			l := log.WithContext("model", "Vos-Test")
			grpcClient := penobjcl.NewObjstoreV1Backend(grpcConn.ClientConn, l)
			now := time.Now()
			viaGrpc(ctx, *bucket, *name, of, grpcClient)
			fmt.Printf("elapsed time is [%v]", time.Since(now))
			return
		}
	case *useDirect:
		{
			tlsc1, err := tlsp.GetClientTLSConfig(globals.Vos)
			if err != nil {
				log.Fatalf("Failed to get TLC config")
			}
			tlsc1.InsecureSkipVerify = true
			dest := "https://node1:" + globals.VosHTTPPort
			viaDirect(dest, *name, tlsc1, of)
		}
	default:
		{

			fr, err := client.GetObject(ctx, *name)
			if err != nil {
				log.Fatalf("could not get object (%s)", err)
			}

			buf := make([]byte, 1024*1024)
			totsize := 0
			now := time.Now()
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
			log.Infof("Downloaded file in [%v]", time.Since(now))
			log.Infof("Got image [%v] of size [%d]", *name, totsize)
		}
	}
}
