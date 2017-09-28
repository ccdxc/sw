package integration

import (
	"context"
	"reflect"
	"sync"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// validateObjectSpec Expects non-pointers in expected and result.
func validateObjectSpec(expected, result interface{}) bool {
	exp := reflect.Indirect(reflect.ValueOf(expected)).FieldByName("Spec").Interface()
	res := reflect.Indirect(reflect.ValueOf(result)).FieldByName("Spec").Interface()

	if !reflect.DeepEqual(exp, res) {
		tinfo.l.Infof("Values are %s[%+v] %s[%+v]", reflect.TypeOf(exp), exp, reflect.TypeOf(res), res)
		return false
	}
	return true
}

func addToWatchList(eventslist *[]kvstore.WatchEvent, obj interface{}, evtype kvstore.WatchEventType) []kvstore.WatchEvent {
	evp := reflect.ValueOf(obj).Interface()
	ev := kvstore.WatchEvent{
		Type:   evtype,
		Object: evp.(runtime.Object),
	}
	return append(*eventslist, ev)

}

func TestCrudOps(t *testing.T) {
	apiserverAddr := "localhost" + ":" + tinfo.apiserverport

	ctx := context.Background()
	// gRPC client
	apicl, err := cache.NewGrpcUpstream(apiserverAddr, tinfo.l)
	if err != nil {
		t.Fatalf("cannot create grpc client")
	}

	// REST Client
	restcl, err := apiclient.NewRestAPIClient("http://localhost:" + tinfo.apigwport)
	if err != nil {
		t.Fatalf("cannot create REST client")
	}

	// Create some objects for use
	var pub, pub2 bookstore.Publisher
	{
		pub = bookstore.Publisher{
			ObjectMeta: api.ObjectMeta{
				Name: "Sahara",
			},
			TypeMeta: api.TypeMeta{
				Kind: "Publisher",
			},
			Spec: bookstore.PublisherSpec{
				Id:      "111",
				Address: "#1 hilane, timbuktoo",
			},
		}
		pub2 = bookstore.Publisher{
			ObjectMeta: api.ObjectMeta{
				Name: "Kalahari",
			},
			TypeMeta: api.TypeMeta{
				Kind: "Publisher",
			},
			Spec: bookstore.PublisherSpec{
				Id:      "112",
				Address: "#2 lowside, timbuktoo",
			},
		}
	}

	// Setup Watcher and slices for validation.
	var rcvWatchEvents, expectWatchEvents []kvstore.WatchEvent
	var wg sync.WaitGroup
	wctx, cancel := context.WithCancel(ctx)
	go func() {
		opts := api.ListWatchOptions{}
		watcher, err := apicl.BookstoreV1().Publisher().Watch(wctx, &opts)
		if err != nil {
			t.Fatalf("Failed to start watch (%s)\n", err)
		}
		orderWatcher, err := apicl.BookstoreV1().Order().Watch(wctx, &opts)
		if err != nil {
			t.Fatalf("Failed to start watch (%s)\n", err)
		}
		ow := true
		pw := true
		for ow && pw {
			select {
			case ev, ok := <-watcher.EventChan():
				t.Logf("received event [%v]", ok)
				if ok {
					t.Logf("  event [%+v]", *ev)
					rcvWatchEvents = append(rcvWatchEvents, *ev)
				} else {
					t.Logf("publisher watcher closed")
					pw = false
				}
			case ev, ok := <-orderWatcher.EventChan():
				t.Logf("received event [%v]", ok)
				if ok {
					t.Logf("  event [%+v]", *ev)
					rcvWatchEvents = append(rcvWatchEvents, *ev)
				} else {
					t.Logf("order watcher closed")
					ow = false
				}
			}
		}
		wg.Done()
	}()
	wg.Add(1)

	// ========= TEST gRPC CRUD Operations ========= //
	t.Logf("test GRPC crud operations")
	{ // --- Create resource via gRPC --- //
		if ret, err := apicl.BookstoreV1().Publisher().Create(ctx, &pub); err != nil {
			t.Fatalf("failed to create publisher(%s)", err)
		} else {
			if !validateObjectSpec(&pub, ret) {
				t.Fatalf("updated object [Add] does not match \n\t[%+v]\n\t[%+v]", pub, ret)
			}
			evp := pub
			expectWatchEvents = addToWatchList(&expectWatchEvents, &evp, kvstore.Created)
		}
	}

	{ // --- Create  a second resource via gRPC ---//
		if ret, err := apicl.BookstoreV1().Publisher().Create(ctx, &pub2); err != nil {
			t.Fatalf("failed to create publisher(%s)", err)
		} else {
			if !validateObjectSpec(&pub2, ret) {
				t.Fatalf("updated object [Add] does not match \n\t[%+v]\n\t[%+v]", pub2, ret)
			}
			evp := pub2
			expectWatchEvents = addToWatchList(&expectWatchEvents, &evp, kvstore.Created)
		}
	}

	{ // --- Get resource via gRPC ---//
		meta := api.ObjectMeta{Name: "Sahara"}
		ret, err := apicl.BookstoreV1().Publisher().Get(ctx, &meta)
		if err != nil {
			t.Fatalf("Error getting publisher (%s)", err)
		}

		t.Logf("Received object %+v", *ret)
		if !validateObjectSpec(&pub, ret) {
			t.Fatalf("updated object [Get] does not match \n\t[%+v]\n\t[%+v]", pub, ret)
		}
	}

	{ // --- List resources via gRPC ---//
		opts := api.ListWatchOptions{}
		pubs, err := apicl.BookstoreV1().Publisher().List(ctx, &opts)
		if err != nil {
			t.Fatalf("List failed %s", err)
		}
		if len(pubs) != 2 {
			t.Fatalf("expected [2] entries, got [%d]", len(pubs))
		}
	}

	{ // --- Duplicate add of the object via gRPC ---//
		if _, err := apicl.BookstoreV1().Publisher().Create(ctx, &pub); err == nil {
			t.Fatalf("Was able to create duplicate publisher")
		}
	}

	{ // --- Update resource via gRPC ---//
		pub.Spec.Address = "#22 hilane, timbuktoo"
		if ret, err := apicl.BookstoreV1().Publisher().Update(ctx, &pub); err != nil {
			t.Fatalf("failed to create publisher(%s)", err)
		} else {
			if !validateObjectSpec(&pub, ret) {
				t.Fatalf("updated object [Add] does not match \n\t[%+v]\n\t[%+v]", pub, ret)
			}
			evp := pub
			expectWatchEvents = addToWatchList(&expectWatchEvents, &evp, kvstore.Updated)
		}
	}

	{ // --- Delete Resource via gRPC --- //
		meta := api.ObjectMeta{Name: "Sahara"}
		ret, err := apicl.BookstoreV1().Publisher().Delete(ctx, &meta)
		if err != nil {
			t.Fatalf("failed to delete publisher(%s)", err)
		}
		if !reflect.DeepEqual(ret.Spec, pub.Spec) {
			t.Fatalf("Deleted object does not match \n\t[%+v]\n\t[%+v]", pub.Spec, ret.Spec)
		}
		evp := pub
		expectWatchEvents = addToWatchList(&expectWatchEvents, &evp, kvstore.Deleted)

	}

	// ========= Test REST CRUD Operations ========= //
	var order1, order2, order2mod bookstore.Order
	{
		order1 = bookstore.Order{
			ObjectMeta: api.ObjectMeta{
				Name: "test for pre-commit hook to generate new Order id - will be overwritten to order-<x> for POST",
			},
			TypeMeta: api.TypeMeta{
				Kind: "Order",
			},
			Spec: bookstore.OrderSpec{
				Id: "order-1",
				Order: []*bookstore.OrderItem{
					{
						ISBNId:   "XXXX",
						Quantity: 1,
					},
				},
			},
		}
		order2 = bookstore.Order{
			ObjectMeta: api.ObjectMeta{
				Name: "test for pre-commit hook to generate new Order id - will be overwritten to order-<x> for POST",
			},
			TypeMeta: api.TypeMeta{
				Kind: "Order",
			},
			Spec: bookstore.OrderSpec{
				Id: "order-2",
				Order: []*bookstore.OrderItem{
					{
						ISBNId:   "YYYY",
						Quantity: 3,
					},
				},
			},
		}
		// Create an new object for modified order 2 - cannot modify same object because watch validation needds original object.
		order2mod = bookstore.Order{
			ObjectMeta: api.ObjectMeta{
				Name: "order-2",
			},
			TypeMeta: api.TypeMeta{
				Kind: "Order",
			},
			Spec: bookstore.OrderSpec{
				Id: "order-2",
				Order: []*bookstore.OrderItem{
					{
						ISBNId:   "XXYY",
						Quantity: 30,
					},
				},
			},
		}
	}

	t.Logf("test REST crud operations")
	{ // ---  POST of the object via REST --- //
		retorder, err := restcl.BookstoreV1().Order().Create(ctx, &order1)
		if err != nil {
			t.Fatalf("Create of Order failed (%s)", err)
		}
		if !reflect.DeepEqual(retorder.Spec, order1.Spec) {
			t.Fatalf("Added Order object does not match \n\t[%+v]\n\t[%+v]", order1.Spec, retorder.Spec)
		}
		evp := order1
		expectWatchEvents = addToWatchList(&expectWatchEvents, &evp, kvstore.Created)
	}

	{ // ---  POST second  object via REST --- //
		retorder, err := restcl.BookstoreV1().Order().Create(ctx, &order2)
		if err != nil {
			t.Fatalf("Create of Order failed (%s)", err)
		}
		if !reflect.DeepEqual(retorder.Spec, order2.Spec) {
			t.Fatalf("Added Order object does not match \n\t[%+v]\n\t[%+v]", order1.Spec, retorder.Spec)
		}
		evp := order2
		expectWatchEvents = addToWatchList(&expectWatchEvents, &evp, kvstore.Created)
	}

	{ // ---  Get  object via REST --- //
		objectMeta := api.ObjectMeta{Name: "order-2"}
		retorder, err := restcl.BookstoreV1().Order().Get(ctx, &objectMeta)
		if err != nil {
			t.Fatalf("failed to get object Order via REST (%s)", err)
		}
		if !validateObjectSpec(retorder, order2) {
			t.Fatalf("updated object [Add] does not match \n\t[%+v]\n\t[%+v]", retorder, order2)
		}
	}

	{ // ---  LIST objects via REST --- //
		opts := api.ListWatchOptions{}
		retlist, err := restcl.BookstoreV1().Order().List(ctx, &opts)
		if err != nil {
			t.Fatalf("List operation returned error (%s)", err)
		}
		if len(retlist) != 2 {
			t.Fatalf("List expecting [2] elements got [%d]", len(retlist))
		}
	}

	{ // ---  PUT objects via REST --- //
		retorder, err := restcl.BookstoreV1().Order().Update(ctx, &order2mod)
		if err != nil {
			t.Fatalf("failed to update object Order via REST (%s)", err)
		}
		if !validateObjectSpec(retorder, order2mod) {
			t.Fatalf("updated object [Update] does not match \n\t[%+v]\n\t[%+v]", retorder, order2)
		}
		evp := order2mod
		expectWatchEvents = addToWatchList(&expectWatchEvents, &evp, kvstore.Updated)
	}

	{ // ---  DELETE objects via REST --- //
		objectMeta := api.ObjectMeta{Name: "order-1"}
		retorder, err := restcl.BookstoreV1().Order().Delete(ctx, &objectMeta)
		if err != nil {
			t.Fatalf("failed to delete object Order via REST (%s)", err)
		}
		if !validateObjectSpec(retorder, order1) {
			t.Fatalf("updated object [Delete] does not match \n\t[%+v]\n\t[%+v]", retorder, order1)
		}
		expectWatchEvents = addToWatchList(&expectWatchEvents, &order1, kvstore.Deleted)
	}

	// ===== Validate Watch Events received === //
	AssertEventually(t,
		func() (bool, []interface{}) { return len(expectWatchEvents) == len(rcvWatchEvents), nil },
		"failed to receive all watch events",
		"10ms",
		"9s")
	cancel()
	wg.Wait()

	for k := range expectWatchEvents {
		if expectWatchEvents[k].Type != rcvWatchEvents[k].Type {
			t.Fatalf("mismatched event type expected (%s) got (%s)", expectWatchEvents[k].Type, rcvWatchEvents[k].Type)
		}
		if !validateObjectSpec(expectWatchEvents[k].Object, rcvWatchEvents[k].Object) {
			t.Fatalf("watch event object [%s] does not match \n\t[%+v]\n\t[%+v]", expectWatchEvents[k].Type, expectWatchEvents[k].Object, rcvWatchEvents[k].Object)
		}
	}
}
