package integration

import (
	"context"
	"fmt"
	"reflect"
	"strconv"
	"strings"
	"sync"
	"testing"
	"time"

	mapset "github.com/deckarep/golang-set"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/venice/apiserver"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
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
	apicl, err := client.NewGrpcUpstream("test", apiserverAddr, tinfo.l)
	if err != nil {
		t.Fatalf("cannot create grpc client")
	}
	defer apicl.Close()

	// REST Client
	restcl, err := apiclient.NewRestAPIClient("http://localhost:" + tinfo.apigwport)
	if err != nil {
		t.Fatalf("cannot create REST client")
	}
	defer restcl.Close()
	// create logged in context
	ctx, err = NewLoggedInContext(ctx, "http://localhost:"+tinfo.apigwport, tinfo.userCred)
	AssertOk(t, err, "cannot create logged in context")

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
				WebAddr: "http://sahara-books.org",
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
				WebAddr: "http://rengtertopian.con:8080/",
			},
		}
	}

	// Setup Watcher and slices for validation.
	// For publisher
	var pRcvWatchEventsMutex sync.Mutex
	var pRcvWatchEvents, pExpectWatchEvents []kvstore.WatchEvent
	// For orders
	var oRcvWatchEventsMutex sync.Mutex
	var oRcvWatchEvents, oExpectWatchEvents []kvstore.WatchEvent
	// For Store
	var sRcvWatchEventsMutex sync.Mutex
	var sRcvWatchEvents, sExpectWatchEvents []kvstore.WatchEvent
	var wg sync.WaitGroup
	wctx, cancel := context.WithCancel(ctx)
	waitWatch := make(chan bool)
	wg.Add(1)
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
		storeWatcher, err := apicl.BookstoreV1().Store().Watch(wctx, &opts)
		if err != nil {
			t.Fatalf("Failed to start watch (%s)\n", err)
		}

		close(waitWatch)
		active := true
		for active {
			select {
			case ev, ok := <-watcher.EventChan():
				t.Logf("ts[%s] received event [%v]", time.Now(), ok)
				if ok {
					t.Logf("  event [%+v]", *ev)
					pRcvWatchEventsMutex.Lock()
					pRcvWatchEvents = append(pRcvWatchEvents, *ev)
					pRcvWatchEventsMutex.Unlock()
				} else {
					t.Logf("publisher watcher closed")
					active = false
				}
			case ev, ok := <-orderWatcher.EventChan():
				t.Logf("ts[%s] received event [%v]", time.Now(), ok)
				if ok {
					t.Logf("  event [%+v]", *ev)
					oRcvWatchEventsMutex.Lock()
					oRcvWatchEvents = append(oRcvWatchEvents, *ev)
					oRcvWatchEventsMutex.Unlock()
				} else {
					t.Logf("order watcher closed")
					active = false
				}
			case ev, ok := <-storeWatcher.EventChan():
				t.Logf("ts[%s] received event [%v]", time.Now(), ok)
				if ok {
					t.Logf("  event [%+v]", *ev)
					sRcvWatchEventsMutex.Lock()
					sRcvWatchEvents = append(sRcvWatchEvents, *ev)
					sRcvWatchEventsMutex.Unlock()
				} else {
					t.Logf("Store watcher closed")
					active = false
				}
			}
		}
		wg.Done()
	}()

	// Wait for watches to be established
	<-waitWatch

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
			pExpectWatchEvents = addToWatchList(&pExpectWatchEvents, &evp, kvstore.Created)
		}
	}

	{ // --- Create  a second resource via gRPC ---//
		if ret, err := apicl.BookstoreV1().Publisher().Create(ctx, &pub2); err != nil {
			t.Fatalf("failed to create publisher(%s)", err)
		} else {
			if !validateObjectSpec(&pub2, ret) {
				t.Fatalf("updated object [Add] does not match \n\t[%+v]\n\t[%+v]", pub2, ret)
			}
			// Verify that the selflink in the objects
			evp := pub2
			pExpectWatchEvents = addToWatchList(&pExpectWatchEvents, &evp, kvstore.Created)
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
			pExpectWatchEvents = addToWatchList(&pExpectWatchEvents, &evp, kvstore.Updated)
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
		pExpectWatchEvents = addToWatchList(&pExpectWatchEvents, &evp, kvstore.Deleted)

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
	{ // Test defaulters //}
		order := &bookstore.Order{}
		if !order.Defaults("v1") {
			t.Errorf("expecting Defaults to return true")
		}
		if order.Spec.Id != "unknown order" {
			t.Fatalf("Not defaulted[%+v]", order)
		}
		t.Logf("received object is [%+v]", order)
		order.Spec.Id = "NonDefault Val"
		if !order.Defaults("v1") {
			t.Errorf("expecting Defaults to return true")
		}
		if order.Spec.Id != "unknown order" {
			t.Fatalf("Not defaulted[%+v]", order)
		}
	}
	{ // ---  POST of the object via REST --- //
		retorder, err := restcl.BookstoreV1().Order().Create(ctx, &order1)
		if err != nil {
			t.Fatalf("Create of Order failed (%s)", err)
		}
		if !reflect.DeepEqual(retorder.Spec, order1.Spec) {
			t.Fatalf("Added Order object does not match \n\t[%+v]\n\t[%+v]", order1.Spec, retorder.Spec)
		}
		evp := order1
		oExpectWatchEvents = addToWatchList(&oExpectWatchEvents, &evp, kvstore.Created)
	}

	{ // ---  POST second  object via REST --- //
		retorder, err := restcl.BookstoreV1().Order().Create(ctx, &order2)
		if err != nil {
			t.Fatalf("Create of Order failed (%s)", err)
		}
		if !reflect.DeepEqual(retorder.Spec, order2.Spec) {
			t.Fatalf("Added Order object does not match \n\t[%+v]\n\t[%+v]", order1.Spec, retorder.Spec)
		}
		selflink := "/v1/bookstore/orders/" + retorder.Name
		if selflink != retorder.SelfLink {
			t.Errorf("Self link does not match expect [%s] got [%s]", selflink, retorder.SelfLink)
		}
		// The Status message should have been overwritten by the API gateway hooks
		if retorder.Status.Message != "Message filled by hook" {
			t.Errorf("API gateway post hook not called [%+v]", retorder)
		}
		evp := order2
		oExpectWatchEvents = addToWatchList(&oExpectWatchEvents, &evp, kvstore.Created)
	}

	{ // ---  POST an object that will be rejected by APIGw hooks --- //
		order3 := order2
		order3.Spec.Id = "order-reject"
		_, err := restcl.BookstoreV1().Order().Create(ctx, &order3)
		if err == nil {
			t.Fatalf("Create of Order should have failed")
		}
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
		oExpectWatchEvents = addToWatchList(&oExpectWatchEvents, &evp, kvstore.Updated)
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
		oExpectWatchEvents = addToWatchList(&oExpectWatchEvents, &order1, kvstore.Deleted)
	}

	// ========= Test Validation and Status update Operations ========= //
	var book1, book1mod bookstore.Book
	{
		book1 = bookstore.Book{
			ObjectMeta: api.ObjectMeta{
				Name: "book1",
			},
			TypeMeta: api.TypeMeta{
				Kind: "book",
			},
			Spec: bookstore.BookSpec{
				ISBNId:   "111-2-31-123456-0",
				Author:   "foo",
				Category: "JunkValue",
			},
		}
		book1mod = bookstore.Book{
			ObjectMeta: api.ObjectMeta{
				Name: "book1",
			},
			TypeMeta: api.TypeMeta{
				Kind: "book",
			},
			Spec: bookstore.BookSpec{
				ISBNId:   "111-2-31-123456-0",
				Author:   "bar",
				Category: "JunkValue2",
			},
			Status: bookstore.BookStatus{
				Inventory: 10,
			},
		}
	}
	{ // Create a new Book entry
		_, err := apicl.BookstoreV1().Book().Create(ctx, &book1)
		if err == nil {
			t.Fatalf("Book create expected to fail due to validation")
		}
		book1.Spec.Defaults("v1")
		retbook, err := apicl.BookstoreV1().Book().Create(ctx, &book1)
		if err != nil {
			t.Fatalf("Book create failed [%s]", err)
		}
		if !reflect.DeepEqual(retbook.Spec, book1.Spec) {
			t.Fatalf("Added Order object does not match \n\t[%+v]\n\t[%+v]", book1.Spec, retbook.Spec)
		}
	}
	{ // Update the Book with status via gRPC with Validation
		_, err := apicl.BookstoreV1().Book().Update(ctx, &book1mod)
		if err == nil {
			t.Fatalf("Validation expected to fail")
		}
		book1mod.Spec.Category = "ChildrensLit"
		_, err = apicl.BookstoreV1().Book().Update(ctx, &book1mod)
		if err != nil {
			t.Fatalf("Expected to succeed")
		}
	}
	{ // Update the Book with Status via REST
		book1mod.Status.Inventory = 100
		_, err = restcl.BookstoreV1().Book().Update(ctx, &book1mod)
		if err != nil {
			t.Fatalf("Failed to update book via REST (%s)", err)
		}
		objectMeta := api.ObjectMeta{Name: "book1"}
		t.Logf("requesting Get for object %+v", objectMeta)
		retbook, err := restcl.BookstoreV1().Book().Get(ctx, &objectMeta)
		if err != nil {
			t.Fatalf("Failed to retrieve book via REST (%s)", err)
		}
		if retbook.Status.Inventory != 10 {
			t.Fatalf("REST update was able to update status fields")
		}
	}

	// ===== Test Operations on Singleton Object ===== //
	{ // Create via the gRPC
		storeObj := bookstore.Store{}
		storeObj.Spec.Contact = "Test Store"
		_, err := apicl.BookstoreV1().Store().Create(ctx, &storeObj)
		if err != nil {
			t.Fatalf("gRPC create of singleton failed (%s)", err)
		}
		evp := storeObj
		sExpectWatchEvents = addToWatchList(&sExpectWatchEvents, &evp, kvstore.Created)
	}

	{ // Create Duplicate via the gRPC
		storeObj := bookstore.Store{}
		storeObj.Spec.Contact = "Test Store2"
		_, err := apicl.BookstoreV1().Store().Create(ctx, &storeObj)
		if err == nil {
			t.Fatalf("gRPC create of duplicate singleton succeded")
		}
	}

	{ // Get via the gRPC
		objectMeta := api.ObjectMeta{Name: "Dummy Store"}
		ret, err := apicl.BookstoreV1().Store().Get(ctx, &objectMeta)
		if err != nil {
			t.Fatalf("gRPC Get of singleton failed")
		}
		if ret.Spec.Contact != "Test Store" {
			t.Fatalf("Singleton object does notmatch exp[Test Store] got [%s]", ret.Spec.Contact)
		}
	}
	{ // Create via the gRPC
		storeObj := bookstore.Store{}
		storeObj.Spec.Contact = "Test Store2"
		_, err := apicl.BookstoreV1().Store().Update(ctx, &storeObj)
		if err != nil {
			t.Fatalf("gRPC create of singleton failed (%s)", err)
		}
		evp := storeObj
		sExpectWatchEvents = addToWatchList(&sExpectWatchEvents, &evp, kvstore.Updated)
	}
	{ // List via the gRPC
		opts := api.ListWatchOptions{}
		ret, err := apicl.BookstoreV1().Store().List(ctx, &opts)
		if err != nil {
			t.Fatalf("gRPC list of singleton failed ")
		}
		if len(ret) != 1 {
			t.Fatalf("Singleton object list got more than one item [%v] ", ret)
		}
		if ret[0].Spec.Contact != "Test Store2" {
			t.Fatalf("Singleton object does notmatch exp[Test Store] got [%s]", ret[0].Spec.Contact)
		}
	}
	{ // Delete via gRPC
		meta := api.ObjectMeta{Name: "Dummy"}
		storeObj, err := apicl.BookstoreV1().Store().Delete(ctx, &meta)
		if err != nil {
			t.Fatalf("Failed to delete singleton object (%s)", err)
		}
		sExpectWatchEvents = addToWatchList(&sExpectWatchEvents, storeObj, kvstore.Deleted)
		opts := api.ListWatchOptions{}
		ret, err := apicl.BookstoreV1().Store().List(ctx, &opts)
		if err != nil {
			t.Fatalf("gRPC List of singleton failed")
		}
		if len(ret) != 0 {
			t.Fatalf("Singleton object list got more than zero item [%v] ", ret)
		}
	}

	// ===== Test Operations on Singleton object via REST ===== //
	{ // Create via the gRPC
		storeObj := bookstore.Store{}
		storeObj.Spec.Contact = "Test Store"
		_, err := restcl.BookstoreV1().Store().Create(ctx, &storeObj)
		if err != nil {
			t.Fatalf("REST create of singleton failed (%s)", err)
		}
		evp := storeObj
		sExpectWatchEvents = addToWatchList(&sExpectWatchEvents, &evp, kvstore.Created)
	}

	{ // Create Duplicate via the gRPC
		storeObj := bookstore.Store{}
		storeObj.Spec.Contact = "Test Store2"
		_, err := restcl.BookstoreV1().Store().Create(ctx, &storeObj)
		if err == nil {
			t.Fatalf("REST create of duplicate singleton succeded")
		}
	}

	{ // Get via the gRPC
		objectMeta := api.ObjectMeta{Name: "Dummy Store"}
		ret, err := restcl.BookstoreV1().Store().Get(ctx, &objectMeta)
		if err != nil {
			t.Fatalf("REST Get of singleton failed")
		}
		if ret.Spec.Contact != "Test Store" {
			t.Fatalf("Singleton object does notmatch exp[Test Store] got [%s]", ret.Spec.Contact)
		}
	}
	{ // Create via the gRPC
		storeObj := bookstore.Store{}
		storeObj.Spec.Contact = "Test Store2"
		_, err := restcl.BookstoreV1().Store().Update(ctx, &storeObj)
		if err != nil {
			t.Fatalf("REST create of singleton failed (%s)", err)
		}
		evp := storeObj
		sExpectWatchEvents = addToWatchList(&sExpectWatchEvents, &evp, kvstore.Updated)
	}
	{ // Delete via gRPC
		meta := api.ObjectMeta{Name: "Dummy"}
		storeObj, err := restcl.BookstoreV1().Store().Delete(ctx, &meta)
		if err != nil {
			t.Fatalf("Failed to delete singleton object (%s)", err)
		}
		sExpectWatchEvents = addToWatchList(&sExpectWatchEvents, storeObj, kvstore.Deleted)
		objectMeta := api.ObjectMeta{Name: "Test Strore2"}
		_, err = restcl.BookstoreV1().Store().Get(ctx, &objectMeta)
		if err == nil {
			t.Fatalf("REST get succeeded after Delete")
		}
	}

	// ===== Validate Watch Events received === //
	AssertEventually(t,
		func() (bool, interface{}) {
			defer pRcvWatchEventsMutex.Unlock()
			pRcvWatchEventsMutex.Lock()
			return len(pExpectWatchEvents) == len(pRcvWatchEvents), nil
		},
		"failed to receive all watch events",
		"10ms",
		"9s")
	AssertEventually(t,
		func() (bool, interface{}) {
			defer oRcvWatchEventsMutex.Unlock()
			oRcvWatchEventsMutex.Lock()
			return len(oExpectWatchEvents) == len(oRcvWatchEvents), nil
		},
		"failed to receive all watch events",
		"10ms",
		"9s")
	AssertEventually(t,
		func() (bool, interface{}) {
			defer sRcvWatchEventsMutex.Unlock()
			sRcvWatchEventsMutex.Lock()
			return len(sExpectWatchEvents) == len(sRcvWatchEvents), nil
		},
		"failed to receive all watch events",
		"10ms",
		"9s")
	cancel()
	wg.Wait()

	for k := range pExpectWatchEvents {
		if pExpectWatchEvents[k].Type != pRcvWatchEvents[k].Type {
			t.Fatalf("mismatched event type expected (%s) got (%s)", pExpectWatchEvents[k].Type, pRcvWatchEvents[k].Type)
		}
		if !validateObjectSpec(pExpectWatchEvents[k].Object, pRcvWatchEvents[k].Object) {
			t.Fatalf("watch event object [%s] does not match \n\t[%+v]\n\t[%+v]", pExpectWatchEvents[k].Type, pExpectWatchEvents[k].Object, pRcvWatchEvents[k].Object)
		}
	}
	for k := range oExpectWatchEvents {
		if oExpectWatchEvents[k].Type != oRcvWatchEvents[k].Type {
			t.Fatalf("mismatched event type expected (%s) got (%s)", oExpectWatchEvents[k].Type, oRcvWatchEvents[k].Type)
		}
		if !validateObjectSpec(oExpectWatchEvents[k].Object, oRcvWatchEvents[k].Object) {
			t.Fatalf("watch event object [%s] does not match \n\t[%+v]\n\t[%+v]", oExpectWatchEvents[k].Type, oExpectWatchEvents[k].Object, oRcvWatchEvents[k].Object)
		}
	}

	for k := range sExpectWatchEvents {
		if sExpectWatchEvents[k].Type != sRcvWatchEvents[k].Type {
			t.Fatalf("mismatched event type expected (%s) got (%s)", sExpectWatchEvents[k].Type, sRcvWatchEvents[k].Type)
		}
		if !validateObjectSpec(sExpectWatchEvents[k].Object, sRcvWatchEvents[k].Object) {
			t.Fatalf("watch event object [%s] does not match \n\t[%+v]\n\t[%+v]", sExpectWatchEvents[k].Type, sExpectWatchEvents[k].Object, sRcvWatchEvents[k].Object)
		}
	}

	// Test Action Functions - gRPC
	actreq := bookstore.ApplyDiscountReq{}
	actreq.Name = "order-2"
	ret, err := apicl.BookstoreV1().Order().Applydiscount(ctx, &actreq)
	if err != nil {
		t.Errorf("gRPC: apply discount action failed (%s)", err)
	}
	if ret.Status.Status != "DISCOUNTED" {
		t.Errorf("gRPC: hook did not run")
	}

	// Reset the object:
	ret, err = apicl.BookstoreV1().Order().Cleardiscount(ctx, &actreq)
	if err != nil {
		t.Errorf("gRPC: clear discount action failed (%s)", err)
	}
	if ret.Status.Status != "PROCESSING" {
		t.Errorf("gRPC: hook did not run")
	}

	// Restock action on Collection
	restockReq := bookstore.RestockRequest{}
	restkresp, err := apicl.BookstoreV1().Book().Restock(ctx, &restockReq)
	if err != nil {
		t.Errorf("gRPC: Restock action failed (%s)", err)
	}
	if restkresp.Count != 3 {
		t.Errorf("gRPC: restock action did not go through hook")
	}

	// Action on singleton object
	outReq := bookstore.OutageRequest{}
	outresp, err := apicl.BookstoreV1().Store().AddOutage(ctx, &outReq)
	if err != nil {
		t.Errorf("gRPC: Outage action failed (%s)", err)
	}
	if len(outresp.Status.CurrentOutages) != 1 {
		t.Errorf("gRPC: Outage action did not go through hook")
	}

	// Test Action Functions - REST
	ret, err = restcl.BookstoreV1().Order().Applydiscount(ctx, &actreq)
	if err != nil {
		t.Errorf("REST: apply discount action failed (%s)", err)
	}
	if ret.Status.Status != "DISCOUNTED" {
		t.Errorf("REST: hook did not run")
	}

	// Restock action on Collection
	restkresp, err = restcl.BookstoreV1().Book().Restock(ctx, &restockReq)
	if err != nil {
		t.Errorf("REST: Restock action failed (%s)", err)
	}
	if restkresp.Count != 3 {
		t.Errorf("REST: restock action did not go through hook")
	}

	// Action on singleton object
	outresp, err = restcl.BookstoreV1().Store().AddOutage(ctx, &outReq)
	if err != nil {
		t.Errorf("REST: Outage action failed (%s)", err)
	}
	if len(outresp.Status.CurrentOutages) != 1 {
		t.Errorf("REST: Outage action did not go through hook")
	}

}

func TestFilters(t *testing.T) {
	apiserverAddr := "localhost" + ":" + tinfo.apiserverport
	ctx := context.Background()

	var book1, book2, book3 bookstore.Book
	{
		book1 = bookstore.Book{
			ObjectMeta: api.ObjectMeta{
				Name:   "selector1",
				Labels: map[string]string{"label1": "xxx", "label2": "yyy"},
			},
			TypeMeta: api.TypeMeta{
				Kind: "book",
			},
			Spec: bookstore.BookSpec{
				ISBNId:   "111-2-31-123456-0",
				Author:   "foo",
				Category: "ChildrensLit",
			},
		}
		book2 = bookstore.Book{
			ObjectMeta: api.ObjectMeta{
				Name:   "selector2",
				Labels: map[string]string{"label1": "xxx", "label2": "ppp"},
			},
			TypeMeta: api.TypeMeta{
				Kind: "book",
			},
			Spec: bookstore.BookSpec{
				ISBNId:   "111-2-31-123456-1",
				Author:   "foo",
				Category: "ChildrensLit",
			},
		}
		book3 = bookstore.Book{
			ObjectMeta: api.ObjectMeta{
				Name:   "selector3",
				Labels: map[string]string{"label3": "qqq"},
			},
			TypeMeta: api.TypeMeta{
				Kind: "book",
			},
			Spec: bookstore.BookSpec{
				ISBNId:   "111-2-31-123456-2",
				Author:   "foo",
				Category: "ChildrensLit",
			},
		}
	}
	// gRPC client
	apicl, err := client.NewGrpcUpstream("test", apiserverAddr, tinfo.l)
	AssertOk(t, err, fmt.Sprintf("cannot create grpc client (%s)", err))
	defer apicl.Close()

	wctx, wcancel := context.WithCancel(ctx)
	opts := []api.ListWatchOptions{
		api.ListWatchOptions{LabelSelector: "label1=xxx"},
		api.ListWatchOptions{LabelSelector: "label1=xxx,label2 in (zzz, ppp)"},
		api.ListWatchOptions{LabelSelector: "label3=xxx"},
		api.ListWatchOptions{FieldChangeSelector: []string{"Spec"}},
		api.ListWatchOptions{FieldChangeSelector: []string{"Status"}},
		api.ListWatchOptions{FieldChangeSelector: []string{"Spec.Category"}},
		api.ListWatchOptions{LabelSelector: "label1=xxx", FieldChangeSelector: []string{"Spec.Category"}},
	}
	var watchers []kvstore.Watcher
	var rwatches [][]kvstore.WatchEvent
	var wmu []sync.Mutex
	var wwg sync.WaitGroup
	watchCount := len(opts)

	watcherFn := func(id int) {
		w := watchers[id]
		defer wwg.Done()
		for {
			select {
			case wev, ok := <-w.EventChan():
				t.Logf("ts[%s] [%d]received event [%v][%+v]", time.Now(), id, ok, wev)
				if ok {
					t.Logf(" W%d event [%+v]", id, *wev)
					wmu[id].Lock()
					rwatches[id] = append(rwatches[id], *wev)
					wmu[id].Unlock()
				} else {
					t.Logf("W%d closed", id)
					return
				}
			}
		}
	}

	validateFn := func(evtype kvstore.WatchEventType, rw []bool, obj *bookstore.Book) bool {
		// Get the last one from each slice.
		failed := false
		for i := 0; i < watchCount; i++ {
			if rw[i] {
				wmu[i].Lock()
				ev := rwatches[i][len(rwatches[i])-1]
				wmu[i].Unlock()
				if ev.Type != evtype {
					t.Errorf("unexpected eventtype [%d] exp: %v got: %v", i, evtype, ev.Type)
					failed = true
				}
				if !validateObjectSpec(obj, ev.Object) {
					t.Errorf("unmatched object [%d] exp:[%+v] got:[%+v]", i, obj, ev.Object)
					failed = true
				}
			}
		}
		return !failed
	}

	waitFn := func(counts []int, exp []bool) {
		for i := 0; i < watchCount; i++ {
			if exp[i] {
				AssertEventually(t,
					func() (bool, interface{}) {
						defer wmu[i].Unlock()
						wmu[i].Lock()
						return len(rwatches[i]) == counts[i]+1, nil
					},
					fmt.Sprintf("[%d]failed to receive all watch events", i),
					"10ms",
					"3s")
			} else {
				AssertConsistently(t, func() (bool, interface{}) {
					defer wmu[i].Unlock()
					wmu[i].Lock()
					return len(rwatches[i]) == counts[i], nil
				}, fmt.Sprintf("inconsistent count on watch[%d] channel exp: %d got: %d", i, counts[i], len(rwatches[i])))
			}
		}
	}
	t.Logf("Delete existing objects")
	// Delete any existing objects before starting watch.
	bl, err := apicl.BookstoreV1().Book().List(ctx, &api.ListWatchOptions{})
	AssertOk(t, err, "error getting list of objects")
	for _, v := range bl {
		meta := &api.ObjectMeta{Name: v.GetName()}
		_, err = apicl.BookstoreV1().Book().Delete(ctx, meta)
		AssertOk(t, err, fmt.Sprintf("error deleting object[%v](%s)", meta, err))
	}
	for i := 0; i < len(opts); i++ {
		w, err := apicl.BookstoreV1().Book().Watch(wctx, &opts[i])
		AssertOk(t, err, fmt.Sprintf("Failed to start watch (%s)", err))
		watchers = append(watchers, w)
		wmu = append(wmu, sync.Mutex{})
		rwatches = append(rwatches, []kvstore.WatchEvent{})
		wwg.Add(1)
		go watcherFn(i)
	}
	book4 := book1
	book4.Spec.Author = "New Author"
	book5 := book4
	book5.Status.Inventory = 4
	book6 := book5
	book6.Spec.Category = "YoungAdult"

	cases := []struct {
		name      string
		oper      kvstore.WatchEventType
		obj       runtime.Object
		expev     []bool
		listRslts []*bookstore.Book
		listOpts  int
	}{
		{name: "case1", oper: kvstore.Created, obj: &book1, expev: []bool{true, false, false, true, true, true, true}},
		{name: "case2", oper: kvstore.Created, obj: &book2, expev: []bool{true, true, false, true, true, true, true}},
		{name: "case3", oper: kvstore.Created, obj: &book3, expev: []bool{false, false, false, true, true, true, false}},
		{name: "case4", oper: kvstore.Updated, obj: &book4, expev: []bool{true, false, false, true, false, false, false}, listOpts: 0, listRslts: []*bookstore.Book{&book1, &book2}},
		{name: "case5", oper: kvstore.Updated, obj: &book5, expev: []bool{true, false, false, false, true, false, false}, listOpts: 1, listRslts: []*bookstore.Book{&book2}},
		{name: "case6", oper: kvstore.Updated, obj: &book6, expev: []bool{true, false, false, true, false, true, true}},
		{name: "case7", oper: kvstore.Deleted, obj: &book6, expev: []bool{true, false, false, true, true, true, true}},
		{name: "case8", oper: kvstore.Deleted, obj: &book2, expev: []bool{true, true, false, true, true, true, true}},
		{name: "case9", oper: kvstore.Deleted, obj: &book3, expev: []bool{false, false, false, true, true, true, false}},
	}
	for _, c := range cases {
		var counts []int
		for i := 0; i < watchCount; i++ {
			counts = append(counts, len(rwatches[i]))
		}
		t.Logf("running case [%s]", c.name)
		obj := c.obj.(*bookstore.Book)
		switch c.oper {
		case kvstore.Created:
			_, err := apicl.BookstoreV1().Book().Create(ctx, obj)
			AssertOk(t, err, "create failed")
		case kvstore.Updated:
			_, err := apicl.BookstoreV1().Book().Update(ctx, obj)
			AssertOk(t, err, "update failed")
		case kvstore.Deleted:
			_, err := apicl.BookstoreV1().Book().Delete(ctx, &obj.ObjectMeta)
			AssertOk(t, err, "delete failed")
		}
		waitFn(counts, c.expev)
		if !validateFn(c.oper, c.expev, obj) {
			t.Fatalf("validate failed")
		}
		if c.listRslts != nil {
			var e, g []interface{}
			objs, err := apicl.BookstoreV1().Book().List(ctx, &opts[c.listOpts])
			AssertOk(t, err, fmt.Sprintf("case [%s] List Failed (%s)", c.name, err))
			for _, v := range c.listRslts {
				e = append(e, v.Name)
			}
			for _, v := range objs {
				g = append(g, v.Name)
			}
			if !mapset.NewSetFromSlice(e).Equal(mapset.NewSetFromSlice(g)) {
				t.Fatalf("-%s : expecting [%v] in list got [%v]", c.name, e, g)
			}
		}

	}
	wcancel()
	wwg.Wait()
}

func TestBatchedWatch(t *testing.T) {
	apiserverAddr := "localhost" + ":" + tinfo.apiserverport

	ctx := context.Background()
	// gRPC client
	apicl, err := client.NewGrpcUpstream("test", apiserverAddr, tinfo.l)
	if err != nil {
		t.Fatalf("cannot create grpc client")
	}
	defer apicl.Close()
	pub := bookstore.Publisher{
		ObjectMeta: api.ObjectMeta{
			Name: "batchSahara",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Publisher",
		},
		Spec: bookstore.PublisherSpec{
			Id:      "111",
			Address: "#1 hilane, timbuktoo",
			WebAddr: "http://sahara-books.org",
		},
	}
	var pRcvWatchEventsMutex sync.Mutex
	var pRcvWatchEvents, pExpectWatchEvents []kvstore.WatchEvent
	var wg sync.WaitGroup
	wctx, cancel := context.WithCancel(ctx)
	waitWatch := make(chan bool)
	wg.Add(1)
	var fromver uint64

	go func() {
		opts := api.ListWatchOptions{}
		watcher, err := apicl.BookstoreV1().Publisher().Watch(wctx, &opts)
		if err != nil {
			t.Fatalf("Failed to start watch (%s)\n", err)
		}
		close(waitWatch)
		active := true
		for active {
			select {
			case ev, ok := <-watcher.EventChan():
				t.Logf("ts[%s] received event [%v]", time.Now(), ok)
				if ok {
					t.Logf("  event [%+v]", *ev)
					p := ev.Object.(*bookstore.Publisher)
					v, _ := strconv.ParseUint(p.ResourceVersion, 10, 64)
					if v <= fromver {
						continue
					}
					pRcvWatchEventsMutex.Lock()
					pRcvWatchEvents = append(pRcvWatchEvents, *ev)
					pRcvWatchEventsMutex.Unlock()
				} else {
					t.Logf("publisher watcher closed")
					active = false
				}
			}
		}
		wg.Done()
	}()
	// Wait for watches to be established
	<-waitWatch

	// List all objects and find fromVer
	{
		objs, _ := apicl.BookstoreV1().Publisher().List(ctx, &api.ListWatchOptions{})
		for _, o := range objs {
			v, _ := strconv.ParseUint(o.ResourceVersion, 10, 64)
			if v > fromver {
				fromver = v
			}
		}
	}

	{ // Insert the object and wait on the Watch event.
		if _, err := apicl.BookstoreV1().Publisher().Create(ctx, &pub); err != nil {
			t.Fatalf("failed to create publisher(%s)", err)
		}
		evp := pub
		pExpectWatchEvents = addToWatchList(&pExpectWatchEvents, &evp, kvstore.Created)

		AssertEventually(t,
			func() (bool, interface{}) {
				defer pRcvWatchEventsMutex.Unlock()
				pRcvWatchEventsMutex.Lock()
				return len(pExpectWatchEvents) == len(pRcvWatchEvents), nil
			},
			"failed to receive all watch events",
			"10ms",
			"3s")
	}
	{ // Update full batch sizes
		for i := 0; i < (apiserver.DefaultWatchBatchSize)*2+2; i++ {
			if _, err := apicl.BookstoreV1().Publisher().Update(ctx, &pub); err != nil {
				t.Fatalf("failed to create publisher(%s)", err)
			}
			evp := pub
			pExpectWatchEvents = addToWatchList(&pExpectWatchEvents, &evp, kvstore.Updated)
		}
		AssertEventually(t,
			func() (bool, interface{}) {
				defer pRcvWatchEventsMutex.Unlock()
				pRcvWatchEventsMutex.Lock()
				return len(pExpectWatchEvents) == len(pRcvWatchEvents), nil
			},
			"failed to receive all watch events",
			"10ms",
			"3s")
	}

	{ // Update full batch sizes in bursts
		for i := 0; i < (apiserver.DefaultWatchBatchSize)*2+2; i++ {
			if i%(apiserver.DefaultWatchBatchSize/2) == 0 {
				time.Sleep(3 * apiserver.DefaultWatchHoldInterval)
			}
			if _, err := apicl.BookstoreV1().Publisher().Update(ctx, &pub); err != nil {
				t.Fatalf("failed to create publisher(%s)", err)
			}
			evp := pub
			pExpectWatchEvents = addToWatchList(&pExpectWatchEvents, &evp, kvstore.Updated)
		}
		AssertEventually(t,
			func() (bool, interface{}) {
				defer pRcvWatchEventsMutex.Unlock()
				pRcvWatchEventsMutex.Lock()
				return len(pExpectWatchEvents) == len(pRcvWatchEvents), nil
			},
			"failed to receive all watch events",
			"10ms",
			"3s")
	}

	cancel()
	wg.Wait()
	for k := range pExpectWatchEvents {
		if pExpectWatchEvents[k].Type != pRcvWatchEvents[k].Type {
			t.Fatalf("mismatched event type expected (%s) got (%s)", pExpectWatchEvents[k].Type, pRcvWatchEvents[k].Type)
		}
		if !validateObjectSpec(pExpectWatchEvents[k].Object, pRcvWatchEvents[k].Object) {
			t.Fatalf("watch event object [%s] does not match \n\t[%+v]\n\t[%+v]", pExpectWatchEvents[k].Type, pExpectWatchEvents[k].Object, pRcvWatchEvents[k].Object)
		}
	}
}

func TestSchemaValidation(t *testing.T) {
	schema := runtime.GetDefaultScheme()

	getField := func(kind, path string) (runtime.Field, bool) {
		var ret runtime.Field
		skip, json, ok := false, true, false
		node := schema.GetSchema(kind)
		if node == nil {
			t.Logf("could not find type Schema %s", kind)
			return runtime.Field{}, false
		}
		// rudimentary tokenize logic.
		tokens := strings.FieldsFunc(path, func(in rune) bool {
			if in == '.' || in == '[' || in == ']' {
				return true
			}
			return false
		})
		if tokens[0] == strings.Title(tokens[0]) {
			json = false
		}

		// Walk the path
		for _, tkn := range tokens {
			if skip == true {
				skip = false
				continue
			}
			if json {
				ret, ok = node.FindFieldByJSONTag(tkn)
			} else {
				ret, ok = node.FindField(tkn)
			}
			if !ok {
				t.Logf("Failed to find [%s]", tkn)
				return ret, ok
			}
			skip = ret.Map || ret.Slice
			if ret.Slice {
				// in case of slice we are already at the destination type.
				//  for all other cases we move to the next type in the path.
				continue
			}
			node = schema.GetSchema(ret.Type)
		}
		return ret, ok
	}

	cases := []struct {
		kind string
		str  string
		exp  bool
	}{
		{kind: "network.Network", str: "Spec", exp: true},
		{kind: "network.Network", str: "Spec.IPv6Subnet", exp: true},
		{kind: "network.Network", str: "Spec.InvalidField", exp: false},
		{kind: "workload.Endpoint", str: "Status.WorkloadAttributes[SOMEKEY]", exp: true},
		// use [*] just to satisfy rudimentary tokenize logic, actual usage skip anyway.
		{kind: "cluster.Cluster", str: "Spec.QuorumNodes[*]", exp: true},
		{kind: "bookstore.Book", str: "Spec.Editions[0].Year", exp: true},
		{kind: "bookstore.Book", str: "Spec.Editions[0].Reviews[*].Date", exp: true},
		{kind: "unknown.Invalid", str: "a.b.c.de", exp: false},

		// JSON Tags
		{kind: "network.Network", str: "spec", exp: true},
		{kind: "network.Network", str: "spec.ipv6-gateway", exp: true},
		{kind: "network.Network", str: "spec.invalid-field", exp: false},
		{kind: "workload.Endpoint", str: "status.workload-attributes[SOMEKEY]", exp: true},
		{kind: "cluster.Cluster", str: "spec.quorum-nodes[*]", exp: true},
		{kind: "bookstore.Book", str: "spec.editions[0].year", exp: true},
		{kind: "bookstore.Book", str: "spec.editions[0].reviews[*].date", exp: true},
		{kind: "unknown.Invalid", str: "a.b[c].d", exp: false},
	}
	for _, c := range cases {
		// We could check the returned field to make sure it is a scalar here.
		_, res := getField(c.kind, c.str)
		if res != c.exp {
			t.Fatalf("expecting [%v] got [%v] %s", c.exp, res, c.str)
		}
	}
}
