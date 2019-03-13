// client is a example client using the generated client bindings.
package main

import (
	"context"
	"flag"
	"fmt"
	"math/rand"
	"net"
	"reflect"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
)

func monitorConns(url, reslvrs string, count int, logger log.Logger) {
	servers := strings.Split(reslvrs, ",")
	if len(servers) < 1 {
		logger.Fatalf("list of resolver addresses needed")
	}
	ropts := []rpckit.Option{}
	tlsp, err := tlsproviders.NewDefaultCMDBasedProvider("node1:9002", globals.APIGw)
	if err != nil {
		log.Fatalf("error getting tls provider (%s)", err)
	}
	ropts = append(ropts, rpckit.WithTLSProvider(tlsp))
	reslvr := resolver.New(&resolver.Config{Name: globals.APIGw, Servers: servers, Options: ropts})
	clnts := make([]apiclient.Services, count)
	for i := 0; i < count; i++ {
		clnts[i], err = apiclient.NewGrpcAPIClient("TestFix", url, logger, rpckit.WithBalancer(balancer.New(reslvr)), rpckit.WithTLSProvider(tlsp))
		if err != nil {
			logger.Fatalf("failed to create API Client")
		}
	}
	errors := make([]error, count)
	ctx := context.Background()
	for {
		<-time.After(3 * time.Second)
		for i := 0; i < len(clnts); i++ {
			_, errors[i] = clnts[i].ClusterV1().Cluster().Get(ctx, &api.ObjectMeta{})
		}
		errs := make([]error, 0)
		ecnt := 0
	ErroLoop:
		for i := 0; i < len(clnts); i++ {
			if errors[i] != nil {
				ecnt++
				for _, e := range errs {
					if errors[i] == e {
						continue ErroLoop
					}
				}
				errs = append(errs, errors[i])
			}
		}
		if ecnt == 0 {
			logger.Infof("No errors found on [%d] clients", count)
			fmt.Printf("[%v]No errors found on [%d] clients\n", time.Now(), count)
		} else {
			logger.Infof("Found [%d] errors on [%d] clients (%s)", ecnt, count, errs)
			fmt.Printf("[%v]Found [%d] errors on [%d] clients (%s)\n", time.Now(), ecnt, count, errs)
		}
	}
}
func main() {
	var (
		grpcaddr  = flag.String("grpc-server", "localhost:9003", "GRPC Port to connect to")
		watch     = flag.Bool("watch", false, "watch publishers")
		sort      = flag.Bool("sort", false, "prep for sort tests")
		resolvers = flag.String("res", "192.168.30.11,192.168.30.12,192.168.30.13", "list of comma seperated resolvers")
		monitor   = flag.Bool("mon", false, "monitor GRPC connections")
		moncount  = flag.Int("count", 10, "monitor connection count")
		out       = flag.String("out", "", "send log output to")
	)
	flag.Parse()

	ctx := context.Background()
	config := log.GetDefaultConfig("GrpcClientExample")
	if *out != "" {
		config.LogToFile = true
		config.LogToStdout = false
		config.FileCfg = log.FileConfig{Filename: *out, MaxSize: 10000000, MaxBackups: 3}
	}
	l := log.SetConfig(config)
	url := *grpcaddr

	var apicl apiclient.Services

	if *monitor {
		monitorConns(*grpcaddr, *resolvers, *moncount, l)
		return
	}

	_, _, err := net.SplitHostPort(*grpcaddr)
	if err != nil {
		opts := []rpckit.Option{}
		servers := strings.Split(*resolvers, ",")
		if len(servers) < 1 {
			l.Fatalf("list of resolver addresses needed")
		}
		reslvr := resolver.New(&resolver.Config{Name: "TestFix", Servers: servers})
		opts = append(opts, rpckit.WithBalancer(balancer.New(reslvr)))
		apicl, err = apiclient.NewGrpcAPIClient("TestFix", *grpcaddr, l, opts...)
		if err != nil {
			l.Fatalf("failed to create API Client")
		}
	} else {
		apicl, err = client.NewGrpcUpstream("example", url, l)
		if err != nil {
			l.Fatalf("Failed to connect to gRPC server [%s]\n", *grpcaddr)
		}
	}

	if *epCreate {
		configureEps(apicl)
		return
	}

	if *createCl {
		clusterCreate(apicl)
		return
	}

	if *sort {
		letters := []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
		genName := func(l int) string {
			r := make([]rune, l)
			for i := range r {
				r[i] = letters[rand.Intn(len(letters))]
			}
			return string(r)
		}

		bl, err := apicl.BookstoreV1().Book().List(ctx, &api.ListWatchOptions{SortOrder: "None"})
		if err != nil {
			fmt.Printf("error getting list of objects (%s)", err)
		}
		for _, v := range bl {
			meta := &api.ObjectMeta{Name: v.GetName()}
			_, err = apicl.BookstoreV1().Book().Delete(ctx, meta)
			if err != nil {
				fmt.Printf("error deleting object[%v](%s)", meta, err)
				return
			}
		}

		names := []string{}
		for i := 0; i < 10; i++ {
			b := bookstore.Book{}
			b.Name = genName(5)
			b.Spec.Category = "ChildrensLit"
			names = append(names, b.Name)
			_, err := apicl.BookstoreV1().Book().Create(ctx, &b)
			if err != nil {
				fmt.Printf("failed to create book [%v](%s)", b.Name, err)
				return
			}
		}

		res := make([][]string, len(api.ListWatchOptions_SortOrders_name))
		for i := 0; i < len(api.ListWatchOptions_SortOrders_name); i++ {
			opts := api.ListWatchOptions{}
			opts.SortOrder = api.ListWatchOptions_SortOrders_name[int32(i)]
			bl, err := apicl.BookstoreV1().Book().List(ctx, &opts)
			if err != nil {
				fmt.Printf("could not list for order [%v]\n", api.ListWatchOptions_SortOrders_name[int32(i)])
				return
			}
			fmt.Printf("[%v] got %d entries\n", api.ListWatchOptions_SortOrders_name[int32(i)], len(bl))
			res[i] = make([]string, len(names))
			for i1, b := range bl {
				res[i][i1] = b.Name
			}
		}
		fmt.Printf("Books names created/listed - \n [NUM] \t %20s ", "created")
		for i := 0; i < len(api.ListWatchOptions_SortOrders_name); i++ {
			fmt.Printf("\t[%20s]", api.ListWatchOptions_SortOrders_name[int32(i)])
		}
		fmt.Printf("\n")
		for i, n := range names {
			fmt.Printf(" [%3d] \t %20s ", i, n)
			for i1 := 0; i1 < len(api.ListWatchOptions_SortOrders_name); i1++ {
				fmt.Printf("\t[%20s]", res[i1][i])
			}
			fmt.Printf("\n")
		}
		return
	}
	if *watch {
		opts := api.ListWatchOptions{FieldSelector: "Spec.Id=112"}
		watcher, err := apicl.BookstoreV1().Publisher().Watch(ctx, &opts)
		if err != nil {
			l.Fatalf("Failed to start watch (%s)\n", err)
		}
		orderWatcher, err := apicl.BookstoreV1().Order().Watch(ctx, &opts)
		if err != nil {
			l.Fatalf("Failed to start watch (%s)\n", err)
		}
		for {
			select {
			case ev, ok := <-watcher.EventChan():
				if ok {
					fmt.Printf("received Publisher Event [ %+v]\n", ev)
				} else {
					fmt.Printf("channel closed!!")
					return
				}
			case ev, ok := <-orderWatcher.EventChan():
				if ok {
					fmt.Printf("received Order Event [ %+v]\n", ev)
				} else {
					fmt.Printf("channel closed!!")
					return
				}
			}
		}
	}
	// Add a Publisher
	var pub = bookstore.Publisher{
		ObjectMeta: api.ObjectMeta{
			Name: "Sahara",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Publisher",
		},
		Spec: bookstore.PublisherSpec{
			Id:      "111",
			Address: "#1 hilane, timbuktoo",
			WebAddr: "http://sahara.org",
		},
	}
	var pub2 = bookstore.Publisher{
		ObjectMeta: api.ObjectMeta{
			Name: "Praire",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Publisher",
		},
		Spec: bookstore.PublisherSpec{
			Id:      "112",
			Address: "#2 hilane, timbuktoo",
			WebAddr: "http://amazon.org",
		},
	}

	meta := api.ObjectMeta{Name: "Sahara"}
	// Delete if there is an existing object
	_, err = apicl.BookstoreV1().Publisher().Delete(ctx, &meta)
	meta.Name = "Praire"
	_, err = apicl.BookstoreV1().Publisher().Delete(ctx, &meta)

	// Create
	ret, err := apicl.BookstoreV1().Publisher().Create(ctx, &pub)
	if err != nil {
		l.Fatalf("failed to create publisher(%s)\n", err)
	}
	if !reflect.DeepEqual(ret.Spec, pub.Spec) {
		l.Fatalf("updated object [Add] does not match \n\t[%+v]\n\t[%+v]\n", pub, ret)
	}
	ret, err = apicl.BookstoreV1().Publisher().Create(ctx, &pub2)
	if err != nil {
		l.Fatalf("failed to create publisher2(%s)\n", err)
	}

	meta.Name = "Sahara"
	ret, err = apicl.BookstoreV1().Publisher().Get(ctx, &meta)
	if err != nil {
		l.Infof("Error getting publisher (%s)", err)
	}
	l.Infof("Received object %+v\n", *ret)
	if !reflect.DeepEqual(ret.Spec, pub.Spec) {
		l.Fatalf("updated object [Get] does not match [%+v] :: [%+v]\n", pub.Spec, ret.Spec)
	}

	opts := api.ListWatchOptions{}
	pubs, err := apicl.BookstoreV1().Publisher().List(ctx, &opts)
	if err != nil {
		l.Fatalf("List failed %s\n", err)
	}
	fmt.Printf("===retrieved list:\n")
	for k, v := range pubs {
		fmt.Printf("---->  %d: %+v\n", k, v)
	}

	// Duplicate add of the object
	l.Infof("Try to Re-add Publisher\n")
	ret, err = apicl.BookstoreV1().Publisher().Create(ctx, &pub)
	if err == nil {
		l.Fatalf("Was able to create duplicate publisher\n")
	}

	// Update the Object
	l.Infof("Update Publisher\n")
	pub.Spec.Address = "#22 hilane, timbuktoo"
	ret, err = apicl.BookstoreV1().Publisher().Update(ctx, &pub)
	if err != nil {
		l.Fatalf("failed to Update publisher(%s)\n", err)
	}
	if !reflect.DeepEqual(ret.Spec, pub.Spec) {
		l.Fatalf("updated object [UPD] does not match \n\t[%+v]\n\t[%+v]\n", pub, ret)
	}

	// Delete the Object
	l.Infof("Delete Publisher\n")
	meta.Name = "Sahara"
	ret, err = apicl.BookstoreV1().Publisher().Delete(ctx, &meta)
	if err != nil {
		l.Fatalf("failed to delete publisher(%s)\n", err)
	}
	if !reflect.DeepEqual(ret.Spec, pub.Spec) {
		l.Fatalf("Deleted object does not match \n\t[%+v]\n\t[%+v]\n", pub.Spec, ret.Spec)
	}

	book := bookstore.Book{
		ObjectMeta: api.ObjectMeta{
			Name: "Book1",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Book",
		},
		Spec: bookstore.BookSpec{
			Category:  "Fiction",
			Publisher: "Praire",
		},
	}
	_, err = apicl.BookstoreV1().Book().Create(ctx, &book)
	if err != nil {
		l.Fatalf("failed to create book (%s)", err)
	}

	store := bookstore.Store{}
	store.Spec.Contact = "Contact name and phone here"
	apicl.BookstoreV1().Store().Delete(ctx, &meta)
	_, err = apicl.BookstoreV1().Store().Create(ctx, &store)
	if err != nil {
		l.Fatalf("failed to create store object ()%s)", err)
	}

	str, err := apicl.BookstoreV1().Store().Get(ctx, &meta)
	if err != nil {
		l.Fatalf("Could not get store object")
	}
	fmt.Printf("Store object is %+v\n", str)

	// Add a tenant
	tenant := cluster.Tenant{
		TypeMeta: api.TypeMeta{
			Kind:       "tenant",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "tenant2",
		},
		Spec: cluster.TenantSpec{
			AdminUser: "admin",
		},
	}

	l.Infof("adding Tenant Object")

	meta.Name = "tenant2"
	// Delete if one exists already
	_, err = apicl.ClusterV1().Tenant().Delete(ctx, &meta)

	// Add now.
	_, err = apicl.ClusterV1().Tenant().Create(ctx, &tenant)
	if err != nil {
		l.Errorf("failed to add tenant object")
	}
}
