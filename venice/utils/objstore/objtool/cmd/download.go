package cmd

import (
	"context"
	"crypto/tls"
	"fmt"
	"io"
	"net/http"
	"os"
	"reflect"
	"strings"
	"time"

	"github.com/spf13/cobra"

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

var (
	useGrpc    bool
	useDirect  bool
	resolvers  string
	provider   string
	useReflect bool
)

var downloadCmd = &cobra.Command{
	Use:   "download",
	Short: "download file from venice <path> <output file>",
	Run:   download,
}

func init() {
	rootCmd.AddCommand(downloadCmd)
	downloadCmd.PersistentFlags().BoolVar(&useGrpc, "grpc", false, "use grpc for operation")
	downloadCmd.PersistentFlags().BoolVar(&useDirect, "direct", false, "use direct minio client for operation")
	downloadCmd.PersistentFlags().StringVar(&resolvers, "resolvers", "node1:9009", "resolvers to use with gRPC")
	downloadCmd.PersistentFlags().StringVar(&provider, "provider", "node1:9002", "TLS Provider to use with gRPC")
}

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
		errorExit(err, "failed to start stream")
	}
	now := time.Now()
	for {
		resp, err := stream.Recv()
		if err == io.EOF {
			fmt.Printf("Downloaded file in [%v]\n", time.Since(now))
			return
		}
		if err != nil {
			errorExit(err, "receive return error")
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
			errorExit(err, "could not write to output")
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
		errorExit(err, "failed to do get on direct endpoint")
	}
	cnt, err := io.Copy(of, resp.Body)
	if err != nil {
		errorExit(err, "Failed to write to file")
	}
	fmt.Printf("downloaded [%d] bytes  to [%v] in [%v]\n", cnt, of.Name(), time.Since(now))
}

func download(cmd *cobra.Command, args []string) {
	if len(args) != 2 {
		errorExit(nil, "need 2 parameters  : <download filename> <destination>")
	}
	name := args[0]
	out := args[1]
	if name == "" || out == "" {
		errorExit(nil, "filename and output file needed")
	}

	if useGrpc && useDirect {
		errorExit(nil, "only one transport override allowed [grpc OR direct]")
	}

	rslvrs := strings.Split(resolvers, ",")
	if len(rslvrs) == 0 {
		errorExit(nil, "resolvers needed for")
	}
	if provider == "" {
		errorExit(nil, "TLS Provider needed")
	}
	opts := []rpckit.Option{}
	tlsp, err := tlsproviders.NewDefaultCMDBasedProvider(provider, "vostest")
	if err != nil {
		errorExit(err, "error getting tls provider")
	}
	opts = append(opts, rpckit.WithTLSProvider(tlsp))
	resolver := resolver.New(&resolver.Config{Name: "TestClient", Servers: rslvrs, Options: opts})
	tlsc := tls.Config{
		InsecureSkipVerify: true,
	}
	client, err := objstore.NewClient(testTenant, bucket, resolver, objstore.WithTLSConfig(&tlsc))
	if err != nil {
		errorExit(err, "could not create client")
	}
	ctx := context.Background()
	of, err := os.Create(out)
	if err != nil {
		log.Fatalf("could not create output file [%s](%s)", out, err)
	}
	defer of.Close()

	switch {
	case useGrpc:
		{
			grpcConn, err := rpckit.NewRPCClient("testclient", globals.Vos, rpckit.WithBalancer(balancer.New(resolver)), rpckit.WithTLSProvider(tlsp))
			if err != nil {
				log.Fatalf("Failed to create grpc client (%s)", err)
			}
			l := log.WithContext("model", "Vos-Test")
			grpcClient := penobjcl.NewObjstoreV1Backend(grpcConn.ClientConn, l)
			now := time.Now()
			viaGrpc(ctx, bucket, name, of, grpcClient)
			fmt.Printf("elapsed time is [%v]\n", time.Since(now))
			return
		}
	case useDirect:
		{
			tlsc1, err := tlsp.GetClientTLSConfig(globals.Vos)
			if err != nil {
				log.Fatalf("Failed to get TLC config")
			}
			tlsc1.InsecureSkipVerify = true
			dest := uri + globals.VosHTTPPort
			viaDirect(dest, name, tlsc1, of)
		}
	default:
		{

			fr, err := client.GetObject(ctx, name)
			if err != nil {
				errorExit(err, "could not get object")
			}

			buf := make([]byte, 1024*1024)
			totsize := 0
			now := time.Now()
			for {
				n, err := fr.Read(buf)
				if err != nil && err != io.EOF {
					errorExit(err, "error while reading object")
				}
				if n == 0 {
					break
				}
				totsize += n
				if _, err = of.Write(buf[:n]); err != nil {
					errorExit(nil, "error writing to output file")
				}
			}
			fmt.Printf("Downloaded file in [%v]\n", time.Since(now))
			fmt.Printf("Got image [%v] of size [%d]\n", name, totsize)
		}
	}
}
