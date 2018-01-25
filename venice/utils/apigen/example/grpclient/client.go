// client is a example client using the generated client bindings.
package main

import (
	"context"
	"flag"
	"fmt"
	"reflect"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/utils/log"
)

func main() {
	var (
		grpcaddr = flag.String("grpc-server", "localhost:9003", "GRPC Port to connect to")
		watch    = flag.Bool("watch", false, "watch publishers")
	)
	flag.Parse()

	ctx := context.Background()
	config := log.GetDefaultConfig("GrpcClientExample")
	l := log.GetNewLogger(config)
	url := *grpcaddr

	apicl, err := client.NewGrpcUpstream(url, l)
	if err != nil {
		l.Fatalf("Failed to connect to gRPC server [%s]\n", *grpcaddr)
	}

	if *watch {
		opts := api.ListWatchOptions{}
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

	// Add a tenant
	tenant := network.Tenant{
		TypeMeta: api.TypeMeta{
			Kind:       "tenant",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant: "tenant2",
			Name:   "tenant2",
		},
		Spec: network.TenantSpec{
			AdminUser: "admin",
		},
	}

	l.Infof("adding Tenant Object")

	meta.Name = "tenant2"
	// Delete if one exists already
	_, err = apicl.TenantV1().Tenant().Delete(ctx, &meta)

	// Add now.
	_, err = apicl.TenantV1().Tenant().Create(ctx, &tenant)
	if err != nil {
		l.Errorf("failed to add tenant object")
	}
}
