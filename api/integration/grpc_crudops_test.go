package integration

import (
	"context"
	"encoding/base64"
	"expvar"
	"fmt"
	"net"
	"os"
	"reflect"
	"strconv"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/deckarep/golang-set"

	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver/pkg"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/api/generated/staging"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
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
	// For a Service Watch
	var srvRcvWatchEventsMutex sync.Mutex
	var srvRcvWatchEvents, srvExpectWatchEvents []kvstore.WatchEvent

	recordWatchEvent := func(eventslist *[]kvstore.WatchEvent, obj interface{}, evtype kvstore.WatchEventType) []kvstore.WatchEvent {
		srvExpectWatchEvents = addToWatchList(&srvExpectWatchEvents, obj, evtype)
		if eventslist != nil {
			return addToWatchList(eventslist, obj, evtype)
		}
		return nil
	}

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
		srvWatcher, err := apicl.BookstoreV1().Watch(wctx, &opts)
		if err != nil {
			t.Fatalf("Failed to start watch (%s)\n", err)
		}
		close(waitWatch)
		active := true
		for active {
			select {
			case ev, ok := <-watcher.EventChan():
				t.Logf("ts[%s] Publisher received event [%v]", time.Now(), ok)
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
				t.Logf("ts[%s] Order received event [%v]", time.Now(), ok)
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
				t.Logf("ts[%s] Store received event [%v]", time.Now(), ok)
				if ok {
					t.Logf("  event [%+v]", *ev)
					sRcvWatchEventsMutex.Lock()
					sRcvWatchEvents = append(sRcvWatchEvents, *ev)
					sRcvWatchEventsMutex.Unlock()
				} else {
					t.Logf("Store watcher closed")
					active = false
				}
			case ev, ok := <-srvWatcher.EventChan():
				t.Logf("ts[%s] Service received event [%v]", time.Now(), ok)
				if ok {
					t.Logf("  event [%+v]", *ev)
					srvRcvWatchEventsMutex.Lock()
					srvRcvWatchEvents = append(srvRcvWatchEvents, *ev)
					srvRcvWatchEventsMutex.Unlock()
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
			pExpectWatchEvents = recordWatchEvent(&pExpectWatchEvents, &evp, kvstore.Created)
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
			pExpectWatchEvents = recordWatchEvent(&pExpectWatchEvents, &evp, kvstore.Created)
		}
	}
	// Wait for the create events to be received
	AssertEventually(t,
		func() (bool, interface{}) {
			defer pRcvWatchEventsMutex.Unlock()
			pRcvWatchEventsMutex.Lock()
			return len(pExpectWatchEvents) == 2, nil
		},
		"failed to receive initial create watch events",
		"10ms",
		"2s")
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
			pExpectWatchEvents = recordWatchEvent(&pExpectWatchEvents, &evp, kvstore.Updated)
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
		pExpectWatchEvents = recordWatchEvent(&pExpectWatchEvents, &evp, kvstore.Deleted)
	}

	// ========= Test REST CRUD Operations ========= //
	var order1, order2, order2mod bookstore.Order
	{
		order1 = bookstore.Order{
			ObjectMeta: api.ObjectMeta{
				Name: "tesForPre-commithookToGenerateNewOrdeId",
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
				Name: "tesForPre-commithookToGenerateNewOrdeId",
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
		or := order1
		or.GenerationID = "3"
		or.UUID = "JUNK"
		or.SelfLink = "Invalid self link"
		or.Status.Status = bookstore.OrderStatus_DISCOUNTED.String()
		or.Status.Message = "override the status"
		retorder, err := restcl.BookstoreV1().Order().Create(ctx, &or)
		if err != nil {
			t.Fatalf("Create of Order failed (%s)", err)
		}
		if !reflect.DeepEqual(retorder.Spec, order1.Spec) {
			t.Fatalf("Added Order object does not match \n\t[%+v]\n\t[%+v]", order1.Spec, retorder.Spec)
		}
		if retorder.GenerationID != "1" {
			t.Fatalf("returned generation id is not 1, got %s", retorder.GenerationID)
		}
		if retorder.Status.Status == bookstore.OrderStatus_DISCOUNTED.String() ||
			retorder.Status.Message == "override the status" ||
			retorder.UUID == "JUNK" ||
			retorder.SelfLink == or.SelfLink {
			t.Fatalf("Status or meta from post was written")
		}
		evp := order1
		oExpectWatchEvents = recordWatchEvent(&oExpectWatchEvents, &evp, kvstore.Created)
	}

	{ // ---  POST second  object via REST --- //
		retorder, err := restcl.BookstoreV1().Order().Create(ctx, &order2)
		if err != nil {
			t.Fatalf("Create of Order failed (%s)", err)
		}
		if !reflect.DeepEqual(retorder.Spec, order2.Spec) {
			t.Fatalf("Added Order object does not match \n\t[%+v]\n\t[%+v]", order1.Spec, retorder.Spec)
		}
		selflink := "/" + globals.ConfigURIPrefix + "/bookstore/v1/orders/" + retorder.Name
		if selflink != retorder.SelfLink {
			t.Errorf("Self link does not match expect [%s] got [%s]", selflink, retorder.SelfLink)
		}
		// The Status message should have been overwritten by the API gateway hooks
		if retorder.Status.Message != "Message filled by hook" {
			t.Errorf("API gateway post hook not called [%+v]", retorder)
		}
		if retorder.GenerationID != "1" {
			t.Fatalf("returned generation id is not 1, got %s", retorder.GenerationID)
		}
		evp := order2
		oExpectWatchEvents = recordWatchEvent(&oExpectWatchEvents, &evp, kvstore.Created)
	}
	// Wait for the create events to be received
	AssertEventually(t,
		func() (bool, interface{}) {
			defer oRcvWatchEventsMutex.Unlock()
			oRcvWatchEventsMutex.Lock()
			return len(oExpectWatchEvents) == 2, nil
		},
		"failed to receive initial create watch events",
		"10ms",
		"2s")
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
		curobj, err := restcl.BookstoreV1().Order().Get(ctx, &order2mod.ObjectMeta)
		if err != nil {
			t.Fatalf("could not get object before PUT (%s)", err)
		}
		or := order2mod
		or.GenerationID = "3"
		or.UUID = "JUNK"
		or.Status.Status = bookstore.OrderStatus_DISCOUNTED.String()
		or.Status.Message = "override the status"
		or.ModTime = curobj.ModTime
		or.SelfLink = "Invalid self link"
		retorder, err := restcl.BookstoreV1().Order().Update(ctx, &or)
		if err != nil {
			t.Fatalf("failed to update object Order via REST (%s)", err)
		}
		if !validateObjectSpec(retorder, order2mod) {
			t.Fatalf("updated object [Update] does not match \n\t[%+v]\n\t[%+v]", retorder, order2)
		}
		if retorder.GenerationID != "2" {
			t.Fatalf("returned generation id is not 2, got [%s]", retorder.GenerationID)
		}
		if retorder.Status.Status == bookstore.OrderStatus_DISCOUNTED.String() ||
			retorder.Status.Message == "override the status" ||
			retorder.UUID == "JUNK" {
			t.Fatalf("Status from post was written")
		}
		if retorder.CreationTime != curobj.CreationTime ||
			retorder.UUID != curobj.UUID ||
			retorder.ModTime == curobj.ModTime ||
			retorder.SelfLink == or.SelfLink {
			t.Fatalf("meta from post was over-written")
		}

		evp := order2mod
		oExpectWatchEvents = recordWatchEvent(&oExpectWatchEvents, &evp, kvstore.Updated)
	}

	{ // put object with invalid Gen ID and overwriting parts of the meta data that are not updatable
		order2mod.GenerationID = "33"
		order2mod.ResourceVersion = "aasdasdada"
		order2mod.UUID = "InvalidUUID"
		order2mod.SelfLink = "/invalid/link"
		objectMeta := api.ObjectMeta{Name: "order-2"}
		oldObj, err := restcl.BookstoreV1().Order().Get(ctx, &objectMeta)
		if err != nil {
			t.Fatalf("failed to get object Order via REST (%s)", err)
		}
		retorder, err := restcl.BookstoreV1().Order().Update(ctx, &order2mod)
		if err != nil {
			t.Fatalf("failed to update object Order via REST (%s)", err)
		}
		if !validateObjectSpec(retorder, order2mod) {
			t.Fatalf("updated object [Update] does not match \n\t[%+v]\n\t[%+v]", retorder, order2)
		}
		if retorder.GenerationID != "3" {
			t.Fatalf("returned generation id is not 3, got [%s]", retorder.GenerationID)
		}
		if retorder.SelfLink != oldObj.SelfLink || retorder.CreationTime != oldObj.CreationTime || retorder.UUID != oldObj.UUID || retorder.ResourceVersion == "aasdasdada" {
			t.Fatalf("parts of the object that should not updatable were updated [%+v][%+v]", retorder.ObjectMeta, oldObj.ObjectMeta)
		}
		evp := order2mod
		oExpectWatchEvents = recordWatchEvent(&oExpectWatchEvents, &evp, kvstore.Updated)
	}

	{ // update via gRPC with invalid SpecGeneration ID
		order2mod.GenerationID = "invalidId"
		order2mod.Status.Status = "PROCESSING"
		retorder, err := apicl.BookstoreV1().Order().Update(ctx, &order2mod)
		if err != nil {
			t.Fatalf("failed to update oder (%s)", err)
		}
		if retorder.GenerationID != "3" {
			t.Fatalf("returned generation id is not 3, got [%s]", retorder.GenerationID)
		}
		evp := order2mod
		oExpectWatchEvents = recordWatchEvent(&oExpectWatchEvents, &evp, kvstore.Updated)
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
		oExpectWatchEvents = recordWatchEvent(&oExpectWatchEvents, &order1, kvstore.Deleted)
	}

	// ========= Test Validation and Status update Operations ========= //
	var book1, book1mod bookstore.Book
	{
		book1 = bookstore.Book{
			ObjectMeta: api.ObjectMeta{
				Name:         "book1",
				GenerationID: "1",
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
				Name:         "book1",
				GenerationID: "1",
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
		book1.Status.Inventory = 22
		retbook, err := apicl.BookstoreV1().Book().Create(ctx, &book1)
		if err != nil {
			t.Fatalf("Book create failed [%s]", err)
		}
		if !reflect.DeepEqual(retbook.Spec, book1.Spec) {
			t.Fatalf("Added Order object does not match \n\t[%+v]\n\t[%+v]", book1.Spec, retbook.Spec)
		}
		if !reflect.DeepEqual(retbook.Status, book1.Status) {
			t.Fatalf("Added Order object Status does not match \n\t[%+v]\n\t[%+v]", book1.Status, retbook.Status)
		}
		evp := book1
		recordWatchEvent(nil, &evp, kvstore.Created)
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
		evp := book1mod
		recordWatchEvent(nil, &evp, kvstore.Updated)
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
		evp := book1mod
		recordWatchEvent(nil, &evp, kvstore.Updated)
	}

	// ===== Test Operations on Singleton Object ===== //
	{ // Create via the gRPC
		storeObj := bookstore.Store{}
		storeObj.Name = "TestStore"
		storeObj.Spec.Contact = "Test Store"
		_, err := apicl.BookstoreV1().Store().Create(ctx, &storeObj)
		if err != nil {
			t.Fatalf("gRPC create of singleton failed (%s)", err)
		}
		evp := storeObj
		sExpectWatchEvents = recordWatchEvent(&sExpectWatchEvents, &evp, kvstore.Created)
	}

	{ // Create Duplicate via the gRPC
		storeObj := bookstore.Store{}
		storeObj.Name = "TestStore"
		storeObj.Spec.Contact = "TestStore2"
		_, err := apicl.BookstoreV1().Store().Create(ctx, &storeObj)
		if err == nil {
			t.Fatalf("gRPC create of duplicate singleton succeded")
		}
	}

	{ // Get via the gRPC
		objectMeta := api.ObjectMeta{Name: "DummyStore"}
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
		storeObj.Name = "TestStore"
		storeObj.Spec.Contact = "Test Store2"
		_, err := apicl.BookstoreV1().Store().Update(ctx, &storeObj)
		if err != nil {
			t.Fatalf("gRPC create of singleton failed (%s)", err)
		}
		evp := storeObj
		sExpectWatchEvents = recordWatchEvent(&sExpectWatchEvents, &evp, kvstore.Updated)
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
		sExpectWatchEvents = recordWatchEvent(&sExpectWatchEvents, storeObj, kvstore.Deleted)
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
		storeObj.Name = "TestStore"
		storeObj.Spec.Contact = "Test Store"
		_, err := restcl.BookstoreV1().Store().Create(ctx, &storeObj)
		if err != nil {
			t.Fatalf("REST create of singleton failed (%s)", err)
		}
		evp := storeObj
		sExpectWatchEvents = recordWatchEvent(&sExpectWatchEvents, &evp, kvstore.Created)
	}

	{ // Create Duplicate via the gRPC
		storeObj := bookstore.Store{}
		storeObj.Name = "TestStore"
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
		storeObj.Name = "TestStore"
		storeObj.Spec.Contact = "Test Store2"
		_, err := restcl.BookstoreV1().Store().Update(ctx, &storeObj)
		if err != nil {
			t.Fatalf("REST create of singleton failed (%s)", err)
		}
		evp := storeObj
		sExpectWatchEvents = recordWatchEvent(&sExpectWatchEvents, &evp, kvstore.Updated)
	}
	{ // Delete via gRPC
		meta := api.ObjectMeta{Name: "Dummy"}
		storeObj, err := restcl.BookstoreV1().Store().Delete(ctx, &meta)
		if err != nil {
			t.Fatalf("Failed to delete singleton object (%s)", err)
		}
		sExpectWatchEvents = recordWatchEvent(&sExpectWatchEvents, storeObj, kvstore.Deleted)
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
	AssertEventually(t,
		func() (bool, interface{}) {
			defer srvRcvWatchEventsMutex.Unlock()
			srvRcvWatchEventsMutex.Lock()
			return len(srvExpectWatchEvents) == len(srvRcvWatchEvents), nil
		},
		fmt.Sprintf("failed to receive all watch events exp: %d got: %d",
			len(srvExpectWatchEvents), len(srvRcvWatchEvents)),
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
				Kind: "Book",
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
				Kind: "Book",
			},
			Spec: bookstore.BookSpec{
				ISBNId:   "111-2-31-123456-1",
				Author:   "bar",
				Category: "ChildrensLit",
			},
		}
		book3 = bookstore.Book{
			ObjectMeta: api.ObjectMeta{
				Name:   "selector3",
				Labels: map[string]string{"label3": "qqq"},
			},
			TypeMeta: api.TypeMeta{
				Kind: "Book",
			},
			Spec: bookstore.BookSpec{
				ISBNId:   "111-2-31-123456-2",
				Author:   "foobar",
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
		api.ListWatchOptions{LabelSelector: "label1=xxx"},                                                 // 0
		api.ListWatchOptions{LabelSelector: "label1=xxx,label2 in (zzz, ppp)"},                            // 1
		api.ListWatchOptions{LabelSelector: "label3=xxx"},                                                 // 2
		api.ListWatchOptions{FieldChangeSelector: []string{"Spec"}},                                       // 3
		api.ListWatchOptions{FieldChangeSelector: []string{"Status"}},                                     // 4
		api.ListWatchOptions{FieldChangeSelector: []string{"Spec.Category"}},                              // 5
		api.ListWatchOptions{LabelSelector: "label1=xxx", FieldChangeSelector: []string{"Spec.Category"}}, // 6
		api.ListWatchOptions{FieldSelector: "name=selector1"},                                             // 7
		api.ListWatchOptions{FieldSelector: "spec.author=foo"},                                            // 8
		api.ListWatchOptions{FieldSelector: "spec.author=New Author"},                                     // 9
		api.ListWatchOptions{FieldSelector: "spec.author in (New Author,foo)"},                            // 10
		api.ListWatchOptions{FieldSelector: "spec.author in (New Author,foo),spec.category=YoungAdult"},   // 11
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

	waitFn := func(name string, counts []int, exp []bool) {
		for i := 0; i < watchCount; i++ {
			if exp[i] {
				AssertEventually(t,
					func() (bool, interface{}) {
						defer wmu[i].Unlock()
						wmu[i].Lock()
						return len(rwatches[i]) == counts[i]+1, nil
					},
					fmt.Sprintf("[%s][%d]failed to receive all watch events got (%d) want (%d)", name, i, len(rwatches[i]), counts[i]+1),
					"10ms",
					"3s")
			} else {
				AssertConsistently(t, func() (bool, interface{}) {
					defer wmu[i].Unlock()
					wmu[i].Lock()
					return len(rwatches[i]) == counts[i], nil
				}, fmt.Sprintf("[%s]inconsistent count on watch[%d] channel exp: %d got: %d", name, i, counts[i], len(rwatches[i])), "10ms", "3s")
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
		{name: "case1", oper: kvstore.Created, obj: &book1, expev: []bool{true, false, false, true, true, true, true, true, true, false, true, false}},
		{name: "case2", oper: kvstore.Created, obj: &book2, expev: []bool{true, true, false, true, true, true, true, false, false, false, false, false}},
		{name: "case3", oper: kvstore.Created, obj: &book3, expev: []bool{false, false, false, true, true, true, false, false, false, false, false, false}},
		{name: "case4", oper: kvstore.Updated, obj: &book4, expev: []bool{true, false, false, true, false, false, false, true, false, true, true, false}, listOpts: 0, listRslts: []*bookstore.Book{&book1, &book2}},
		{name: "case5", oper: kvstore.Updated, obj: &book5, expev: []bool{true, false, false, false, true, false, false, true, false, true, true, false}, listOpts: 1, listRslts: []*bookstore.Book{&book2}},
		{name: "case6", oper: kvstore.Updated, obj: &book6, expev: []bool{true, false, false, true, false, true, true, true, false, true, true, true}, listOpts: 11, listRslts: []*bookstore.Book{&book6}},
		{name: "case7", oper: kvstore.Deleted, obj: &book6, expev: []bool{true, false, false, true, true, true, true, true, false, true, true, true}},
		{name: "case8", oper: kvstore.Deleted, obj: &book2, expev: []bool{true, true, false, true, true, true, true, false, false, false, false, false}},
		{name: "case9", oper: kvstore.Deleted, obj: &book3, expev: []bool{false, false, false, true, true, true, false, false, false, false, false, false}},
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
		waitFn(c.name, counts, c.expev)
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

	getField := func(kind, path string) (api.Field, bool) {
		var ret api.Field
		skip, json, ok := false, true, false
		node := schema.GetSchema(kind)
		if node == nil {
			t.Logf("could not find type Schema %s", kind)
			return api.Field{}, false
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

func TestStaging(t *testing.T) {
	ctx := context.Background()

	// REST Client
	restcl, err := apiclient.NewRestAPIClient("http://localhost:" + tinfo.apigwport)
	if err != nil {
		t.Fatalf("cannot create REST client")
	}
	defer restcl.Close()

	ctx, err = NewLoggedInContext(ctx, "http://localhost:"+tinfo.apigwport, tinfo.userCred)
	AssertOk(t, err, "cannot create logged in context")

	apiserverAddr := "localhost" + ":" + tinfo.apiserverport
	apicl, err := client.NewGrpcUpstream("test", apiserverAddr, tinfo.l)
	if err != nil {
		t.Fatalf("cannot create grpc client")
	}
	defer apicl.Close()

	var (
		bufName    = "TestBuffer"
		tenantName = "default"
	)
	{ // Create a buffer
		buf := staging.Buffer{}
		buf.Name = bufName
		buf.Tenant = tenantName
		_, err := restcl.StagingV1().Buffer().Create(ctx, &buf)
		if err != nil {
			t.Fatalf("failed to create staging buffer %s", err)
		}
	}

	// Staging Client
	stagecl, err := apiclient.NewStagedRestAPIClient("http://localhost:"+tinfo.apigwport, bufName)
	if err != nil {
		t.Fatalf("cannot create Staged REST client")
	}
	defer stagecl.Close()

	var customers []bookstore.Customer
	{
		customers = []bookstore.Customer{
			{
				ObjectMeta: api.ObjectMeta{
					Name:     "customer1",
					SelfLink: "/junk",
				},
				TypeMeta: api.TypeMeta{
					Kind: "Customer",
				},
				Spec: bookstore.CustomerSpec{
					Address:  "1111 Wherewhich lane",
					Password: []byte("Test123"),
				},
			},
			{
				ObjectMeta: api.ObjectMeta{
					Name: "customer2",
				},
				TypeMeta: api.TypeMeta{
					Kind: "Customer",
				},
				Spec: bookstore.CustomerSpec{
					Address:  "1112 Wherewhich lane",
					Password: []byte("Test123"),
				},
			},
			{
				ObjectMeta: api.ObjectMeta{
					Name: "customer3",
				},
				TypeMeta: api.TypeMeta{
					Kind: "Customer",
				},
				Spec: bookstore.CustomerSpec{
					Address:  "1113 Wherewhich lane",
					Password: []byte("Test123"),
				},
			},
			{
				ObjectMeta: api.ObjectMeta{
					Name: "customer4",
				},
				TypeMeta: api.TypeMeta{
					Kind: "Customer",
				},
				Spec: bookstore.CustomerSpec{
					Address:  "1114 Wherewhich lane",
					Password: []byte("Test123"),
				},
			},
		}

	}
	var custName string
	{ // Stage an object
		retcust, err := stagecl.BookstoreV1().Customer().Create(ctx, &customers[0])
		if err != nil {
			t.Fatalf("Create of customer failed (%s)", err)
		}
		custName = retcust.Name
	}

	{ // Get staged object from staging buffer
		objectMeta := api.ObjectMeta{Name: custName}
		_, err := stagecl.BookstoreV1().Customer().Get(ctx, &objectMeta)
		if err != nil {
			t.Fatalf("Get of Order from staged buffer failed (%s)", err)
		}
	}
	{ // Get staged object without staging
		objectMeta := api.ObjectMeta{Name: custName}
		_, err := restcl.BookstoreV1().Customer().Get(ctx, &objectMeta)
		if err == nil {
			t.Fatalf("Get of Order unstaged cache succeeded ")
		}
	}
	{ // get staging buffer
		opts := api.ObjectMeta{
			Tenant: tenantName,
			Name:   bufName,
		}

		buf, err := restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != 1 {
			t.Fatalf("expecting [1] item found [%d]", len(buf.Status.Items))
		}
		if len(buf.Status.Errors) != 0 {
			t.Fatalf("expecting [] verification errors found [%d] [%v][%v]", len(buf.Status.Errors), buf.Status.Errors, buf.Status.Items)
		}
		obj := buf.Status.Items[0]
		if obj.Method != "create" || obj.URI != "/configs/bookstore/v1/customers/"+custName {
			t.Fatalf("object received does not match %+v", obj)
		}
	}
	{ // Clear full buffer
		opts := api.ObjectMeta{
			Tenant: tenantName,
			Name:   bufName,
		}
		in := staging.ClearAction{}
		in.Tenant = tenantName
		in.Name = bufName
		_, err := restcl.StagingV1().Buffer().Clear(ctx, &in)
		if err != nil {
			t.Fatalf("failed to Clear staging buffer %s", err)
		}
		buf, err := restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != 0 {
			t.Fatalf("expecting [0] item found [%d]", len(buf.Status.Items))
		}
		if len(buf.Status.Errors) != 0 {
			t.Fatalf("expecting [] verification errors found [%d] [%v]", len(buf.Status.Errors), buf.Status.Errors)
		}
	}
	{ // selective clear of objects
		opts := api.ObjectMeta{
			Tenant: tenantName,
			Name:   bufName,
		}
		names := []string{}
		for i := 1; i < 4; i++ {
			retcust, err := stagecl.BookstoreV1().Customer().Create(ctx, &customers[i])
			if err != nil {
				t.Fatalf("Create of Order failed (%s)", err)
			}
			names = append(names, retcust.Name)
		}
		buf, err := restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != 3 {
			t.Fatalf("expecting [3] item found [%d]", len(buf.Status.Items))
		}
		if len(buf.Status.Errors) != 0 {
			t.Fatalf("expecting [] verification errors found [%d] [%v]", len(buf.Status.Errors), buf.Status.Errors)
		}
		uris := []string{}
		for _, v := range buf.Status.Items {
			uris = append(uris, v.URI)
			for nk, n := range names {
				if strings.HasSuffix(v.URI, n) {
					names[nk] = "SHOULDNOTEXIST###"
				}
			}
		}
		for _, n := range names {
			if n != "SHOULDNOTEXIST###" {
				t.Fatalf("Was not expecting object in returned buffer [%s]", n)
			}
		}

		ca := staging.ClearAction{}
		ca.Tenant = tenantName
		ca.Name = bufName
		ca.Spec.Items = []*staging.ItemId{
			&staging.ItemId{
				URI: uris[1],
			},
		}
		_, err = restcl.StagingV1().Buffer().Clear(ctx, &ca)
		if err != nil {
			t.Fatalf("failed to clear specific items in buffer (%s)", err)
		}
		buf, err = restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != 2 {
			t.Fatalf("expecting [2] item found [%d]", len(buf.Status.Items))
		}
		if len(buf.Status.Errors) != 0 {
			t.Fatalf("expecting [] verification errors found [%d] [%v]", len(buf.Status.Errors), buf.Status.Errors)
		}
	}
	{ // commit the buffer and verify that it is accessible in unstaged path.
		opts := api.ObjectMeta{
			Tenant: tenantName,
			Name:   bufName,
		}
		buf, err := restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != 2 {
			t.Fatalf("expecting [2] item found [%d]", len(buf.Status.Items))
		}
		if len(buf.Status.Errors) != 0 {
			t.Fatalf("expecting [] verification errors found [%d] [%v]", len(buf.Status.Errors), buf.Status.Errors)
		}
		names := []string{}
		for _, v := range buf.Status.Items {
			ns := strings.Split(v.URI, "/")
			names = append(names, ns[len(ns)-1])
		}
		ca := staging.CommitAction{}
		ca.Name = bufName
		ca.Tenant = tenantName
		cresp, err := restcl.StagingV1().Buffer().Commit(ctx, &ca)
		if err != nil {
			t.Fatalf("failed to commit staging buffer (%s)", err)
		}
		if cresp.Status.Status != staging.CommitActionStatus_SUCCESS.String() {
			t.Fatalf("commit operation failed %v", cresp.Status)
		}
		buf, err = restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != 0 {
			t.Fatalf("expecting [0] item found [%d]", len(buf.Status.Items))
		}
		if len(buf.Status.Errors) != 0 {
			t.Fatalf("expecting [] verification errors found [%d] [%v]", len(buf.Status.Errors), buf.Status.Errors)
		}
		// Get non-staged object
		for _, v := range names {
			objectMeta := api.ObjectMeta{Name: v}
			_, err := restcl.BookstoreV1().Customer().Get(ctx, &objectMeta)
			if err != nil {
				t.Fatalf("Get of Order %s failed after commit (%s)", v, err)
			}
			cust, _ := apicl.BookstoreV1().Customer().Get(ctx, &objectMeta)
			decpass, _ := base64.StdEncoding.DecodeString(string(cust.Spec.Password))
			if string(cust.Spec.Password) != "Test123" {
				t.Fatalf("expected password (%s), got (%s), base64 decoded (%s), for customer (%s)", "Test123", string(cust.Spec.Password), decpass, cust.Name)
			}
		}
	}
	{ // Modifying existing object via staging
		lopts := api.ListWatchOptions{}
		lst, err := restcl.BookstoreV1().Customer().List(ctx, &lopts)
		if len(lst) != 2 {
			t.Fatalf("expecting 2 objects in list, got %d", len(lst))
		}
		names := []string{}
		for _, v := range lst {
			v.Spec.Address = "not at " + v.Spec.Address
			v.Spec.Password = []byte("Test123")
			retcust, err := stagecl.BookstoreV1().Customer().Update(ctx, v)
			if err != nil {
				t.Fatalf("update of Order failed (%s)", err)
			}
			names = append(names, retcust.Name)
		}
		opts := api.ObjectMeta{
			Tenant: tenantName,
			Name:   bufName,
		}
		buf, err := restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != len(names) {
			t.Fatalf("expecting [2] item found [%d]", len(buf.Status.Items))
		}
		if len(buf.Status.Errors) != 0 {
			t.Fatalf("expecting [] verification errors found [%d] [%v]", len(buf.Status.Errors), buf.Status.Errors)
		}
		ca := staging.CommitAction{}
		ca.Name = bufName
		ca.Tenant = tenantName
		cresp, err := restcl.StagingV1().Buffer().Commit(ctx, &ca)
		if err != nil {
			t.Fatalf("failed to commit staging buffer (%s)", err)
		}
		if cresp.Status.Status != staging.CommitActionStatus_SUCCESS.String() {
			t.Fatalf("commit operation failed %v", cresp.Status)
		}
		buf, err = restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != 0 {
			t.Fatalf("expecting [0] item found [%d]", len(buf.Status.Items))
		}
		if len(buf.Status.Errors) != 0 {
			t.Fatalf("expecting [] verification errors found [%d] [%v]", len(buf.Status.Errors), buf.Status.Errors)
		}
		// Get non-staged object
		for _, v := range names {
			objectMeta := api.ObjectMeta{Name: v}
			retObj, err := restcl.BookstoreV1().Customer().Get(ctx, &objectMeta)
			if err != nil {
				t.Fatalf("Get of Order %s failed after commit (%s)", v, err)
			}
			if !strings.Contains(retObj.Spec.Address, "not at") {
				t.Fatalf("object not updated in kvstore [%s] [%+v]", v, retObj)
			}
		}
	}
	{ // delete existing objects via staging
		lopts := api.ListWatchOptions{}
		lst, err := restcl.BookstoreV1().Customer().List(ctx, &lopts)
		if len(lst) != 2 {
			t.Fatalf("expecting 2 objects in list, got %d", len(lst))
		}
		names := []string{}
		for _, v := range lst {
			meta := api.ObjectMeta{Name: v.Name}
			retcust, err := stagecl.BookstoreV1().Customer().Delete(ctx, &meta)
			if err != nil {
				t.Fatalf("delete of Order failed (%s)", err)
			}
			names = append(names, retcust.Name)
		}
		opts := api.ObjectMeta{
			Tenant: tenantName,
			Name:   bufName,
		}
		buf, err := restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != len(names) {
			t.Fatalf("expecting [2] item found [%d]", len(buf.Status.Items))
		}
		if len(buf.Status.Errors) != 0 {
			t.Fatalf("expecting [] verification errors found [%d] [%v]", len(buf.Status.Errors), buf.Status.Errors)
		}
		ca := staging.CommitAction{}
		ca.Name = bufName
		ca.Tenant = tenantName
		cresp, err := restcl.StagingV1().Buffer().Commit(ctx, &ca)
		if err != nil {
			t.Fatalf("failed to commit staging buffer (%s)", err)
		}
		if cresp.Status.Status != staging.CommitActionStatus_SUCCESS.String() {
			t.Fatalf("commit operation failed %v", cresp.Status)
		}
		buf, err = restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != 0 {
			t.Fatalf("expecting [0] item found [%d]", len(buf.Status.Items))
		}
		if len(buf.Status.Errors) != 0 {
			t.Fatalf("expecting [] verification errors found [%d] [%v]", len(buf.Status.Errors), buf.Status.Errors)
		}
		lst, err = restcl.BookstoreV1().Customer().List(ctx, &lopts)
		// Get non-staged object
		for _, v := range names {
			objectMeta := api.ObjectMeta{Name: v}
			_, err := restcl.BookstoreV1().Customer().Get(ctx, &objectMeta)
			if err == nil {
				t.Fatalf("found object after deletion  %s", v)
			}
		}
	}

	{ // Staging request that should fail
		opts := api.ObjectMeta{
			Tenant: tenantName,
			Name:   bufName,
		}
		lopts := api.ListWatchOptions{}
		names := []string{}
		for i := 0; i < 4; i++ {
			retcust, err := stagecl.BookstoreV1().Customer().Create(ctx, &customers[i])
			if err != nil {
				t.Fatalf("Create of Object failed (%s)", err)
			}
			names = append(names, retcust.Name)
		}

		buf, err := restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != 4 {
			t.Fatalf("expecting [4] item found [%d]", len(buf.Status.Items))
		}

		// Create one of the objects directly without staging.
		_, err = restcl.BookstoreV1().Customer().Create(ctx, &customers[1])
		if err != nil {
			t.Fatalf("Create of object directly failed (%s)", err)
		}
		// Now commit staged object
		ca := staging.CommitAction{}
		ca.Name = bufName
		ca.Tenant = tenantName
		cresp, err := restcl.StagingV1().Buffer().Commit(ctx, &ca)
		if err != nil {
			t.Fatalf("commit operation failed (%s)", err)
		}
		if cresp.Status.Status == staging.CommitActionStatus_SUCCESS.String() {
			t.Fatalf("commit operation succeeded, expected to fail")
		}
		lst, err := restcl.BookstoreV1().Customer().List(ctx, &lopts)
		if err != nil {
			t.Fatalf("failed to retrieve list of objects")
		}
		if len(lst) != 1 {
			t.Fatalf("expecting 1 objects, found %d", len(lst))
		}
		if lst[0].Name != customers[1].Name {
			t.Fatalf("Expecting %v got %v object", lst[0].Name, customers[1].Name)
		}
	}

	{ // Test consistent update
		// Update status via grpc
		copts := api.ObjectMeta{Name: "customer2"}
		opts := api.ObjectMeta{
			Tenant: tenantName,
			Name:   bufName,
		}
		cobj, err := restcl.BookstoreV1().Customer().Get(ctx, &copts)
		if err != nil {
			t.Fatalf("failed to get object (%s)", err)
		}
		cobj.Status.AccountStatus = "active"
		cobj.Spec.Password = []byte("Test123")
		_, err = apicl.BookstoreV1().Customer().Update(ctx, cobj)
		if err != nil {
			t.Fatalf("failed to update object (%s)", err)
		}
		customers[1].Spec.Address = "Updated New Address"
		customers[1].Spec.Password = []byte("Test123")
		_, err = stagecl.BookstoreV1().Customer().Update(ctx, &customers[1])
		if err != nil {
			t.Fatalf("failed to stage update operation *%s)", err)
		}
		buf, err := restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != 4 {
			t.Fatalf("expecting [4] item found [%d]", len(buf.Status.Items))
		}

		// Now commit staged object
		ca := staging.CommitAction{}
		ca.Name = bufName
		ca.Tenant = tenantName
		cresp, err := restcl.StagingV1().Buffer().Commit(ctx, &ca)
		if err != nil {
			t.Fatalf("commit operation failed (%s)", err)
		}
		if cresp.Status.Status != staging.CommitActionStatus_SUCCESS.String() {
			t.Fatalf("commit operation expected to succeeded got (%v)", cresp.Status)
		}
		cobj, err = restcl.BookstoreV1().Customer().Get(ctx, &copts)
		if err != nil {
			t.Fatalf("failed to get object (%s)", err)
		}
		if cobj.Spec.Address != "Updated New Address" || cobj.Status.AccountStatus != "active" {
			t.Fatalf("consisten update failed [%v]", cobj)
		}
	}
	{ // delete all objects
		lopts := api.ListWatchOptions{}
		lst, err := restcl.BookstoreV1().Customer().List(ctx, &lopts)
		if len(lst) != 4 {
			t.Fatalf("expecting 2 objects in list, got %d", len(lst))
		}
		names := []string{}
		for _, v := range lst {
			meta := api.ObjectMeta{Name: v.Name}
			retcust, err := stagecl.BookstoreV1().Customer().Delete(ctx, &meta)
			if err != nil {
				t.Fatalf("delete of Order failed (%s)", err)
			}
			names = append(names, retcust.Name)
		}
		opts := api.ObjectMeta{
			Tenant: tenantName,
			Name:   bufName,
		}
		buf, err := restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != len(names) {
			t.Fatalf("expecting [2] item found [%d]", len(buf.Status.Items))
		}
		if len(buf.Status.Errors) != 0 {
			t.Fatalf("expecting [] verification errors found [%d] [%v]", len(buf.Status.Errors), buf.Status.Errors)
		}
		ca := staging.CommitAction{}
		ca.Name = bufName
		ca.Tenant = tenantName
		cresp, err := restcl.StagingV1().Buffer().Commit(ctx, &ca)
		if err != nil {
			t.Fatalf("failed to commit staging buffer (%s)", err)
		}
		if cresp.Status.Status != staging.CommitActionStatus_SUCCESS.String() {
			t.Fatalf("commit operation failed %v", cresp.Status)
		}
		buf, err = restcl.StagingV1().Buffer().Get(ctx, &opts)
		if err != nil {
			t.Fatalf("failed to get staging buffer %s", err)
		}
		if len(buf.Status.Items) != 0 {
			t.Fatalf("expecting [0] item found [%d]", len(buf.Status.Items))
		}
		if len(buf.Status.Errors) != 0 {
			t.Fatalf("expecting [] verification errors found [%d] [%v]", len(buf.Status.Errors), buf.Status.Errors)
		}
		lst, err = restcl.BookstoreV1().Customer().List(ctx, &lopts)
		if len(lst) > 0 {
			t.Fatalf("found customer object after delete [%+v]", lst)
		}
		// Get non-staged object
		for _, v := range names {
			objectMeta := api.ObjectMeta{Name: v}
			_, err := restcl.BookstoreV1().Customer().Get(ctx, &objectMeta)
			if err == nil {
				t.Fatalf("found object after deletion  %s", v)
			}
		}
	}
	{ // test restore path
		lopts := api.ListWatchOptions{}
		lst, err := restcl.BookstoreV1().Customer().List(ctx, &lopts)
		AssertOk(t, err, "failed to list object via rest (%s)", err)
		for _, v := range lst {
			fmt.Printf("object before restore is [%+v]\n", v)
		}
		customers[0].Spec.Address = "This is just before restore"
		_, err = restcl.BookstoreV1().Customer().Create(ctx, &customers[0])
		AssertOk(t, err, "failed to create object via rest (%s)", err)
		_, err = restcl.BookstoreV1().Customer().Create(ctx, &customers[1])
		AssertOk(t, err, "failed to create object via rest (%s)", err)
		_, err = stagecl.BookstoreV1().Customer().Create(ctx, &customers[2])
		AssertOk(t, err, "failed to create object via staging (%s)", err)
		changedCustomer := customers[0]
		changedCustomer.Spec.Address = "changed address"
		_, err = stagecl.BookstoreV1().Customer().Update(ctx, &changedCustomer)
		AssertOk(t, err, "failed to update object via staging (%s)", err)
		_, err = stagecl.BookstoreV1().Customer().Delete(ctx, &customers[1].ObjectMeta)
		AssertOk(t, err, "failed to delete object via staging (%s)", err)
		opts := api.ObjectMeta{
			Tenant: tenantName,
			Name:   bufName,
		}
		bufobj, err := restcl.StagingV1().Buffer().Get(ctx, &opts)
		AssertOk(t, err, "failed to get staging buffer (%s)", err)
		apisrv := apisrvpkg.MustGetAPIServer()
		apisrv.Stop()
		gw := apigwpkg.MustGetAPIGateway()
		gw.Stop()
		// allow ports to close
		time.Sleep(time.Second)
		config := tinfo.apisrvConfig
		// config.GrpcServerPort = ":" + tinfo.apiserverport
		go apisrv.Run(config)
		apisrv.WaitRunning()
		addr, err := apisrv.GetAddr()
		if err != nil {
			t.Fatalf("could not get apiserver address after restart(%s)", err)
		}
		_, port, err := net.SplitHostPort(addr)
		if err != nil {
			os.Exit(-1)
		}
		tinfo.apiserverport = port
		gwconfig := tinfo.gwConfig
		gwconfig.BackendOverride["pen-apiserver"] = "localhost:" + port
		go gw.Run(gwconfig)
		gw.WaitRunning()
		gwaddr, err := gw.GetAddr()
		if err != nil {
			os.Exit(-1)
		}
		_, port, err = net.SplitHostPort(gwaddr.String())
		if err != nil {
			os.Exit(-1)
		}
		tinfo.apigwport = port
		tinfo.cache = apisrvpkg.GetAPIServerCache()
		restcl.Close()
		apicl.Close()
		restcl, err = apiclient.NewRestAPIClient("http://localhost:" + tinfo.apigwport)
		if err != nil {
			t.Fatalf("cannot create REST client")
		}
		defer restcl.Close()

		apiserverAddr := "localhost" + ":" + tinfo.apiserverport
		apicl, err = client.NewGrpcUpstream("test", apiserverAddr, tinfo.l)
		if err != nil {
			t.Fatalf("cannot create grpc client")
		}
		defer apicl.Close()

		AssertEventually(t, func() (bool, interface{}) {
			ctx, err = NewLoggedInContext(ctx, "http://localhost:"+tinfo.apigwport, tinfo.userCred)
			return err == nil, nil
		}, "failed to get logged in context after restart", "10ms", "5s")

		var bufobj1 *staging.Buffer
		AssertEventually(t, func() (bool, interface{}) {
			bufobj1, err = restcl.StagingV1().Buffer().Get(ctx, &opts)
			return err == nil, nil
		}, "failed to get buffer after restart", "10ms", "3s")
		Assert(t, len(bufobj.Status.Items) == len(bufobj1.Status.Items), "number of items in buffer does not match got[%d] want [%d]", len(bufobj1.Status.Items), len(bufobj.Status.Items))
		itemMap := make(map[string]staging.Item)
		for _, v := range bufobj.Status.Items {
			itemMap[v.ItemId.URI] = staging.Item{ItemId: v.ItemId}
		}
		for _, v := range bufobj1.Status.Items {
			v1, ok := itemMap[v.ItemId.URI]
			Assert(t, ok, "unknown key after restore[%v]", v.ItemId.URI)
			Assert(t, reflect.DeepEqual(v1.ItemId, v.ItemId), "restored buffer item does not match\n[ got]:[%+v]\n[want]:[%+v]\n", v, v1)
		}
		lst, err = restcl.BookstoreV1().Customer().List(ctx, &lopts)
		AssertOk(t, err, "failed to list via rest (%s)", err)
		for _, v := range lst {
			vopts := api.ObjectMeta{Name: v.Name}
			_, err := restcl.BookstoreV1().Customer().Delete(ctx, &vopts)
			AssertOk(t, err, "failed to delete via rest (%s)", err)
		}
	}
	{ // Delete staging object
		objMeta := api.ObjectMeta{}
		objMeta.Name = bufName
		objMeta.Tenant = tenantName
		_, err := restcl.StagingV1().Buffer().Delete(ctx, &objMeta)
		if err != nil {
			t.Fatalf("failed to delete staging buffer %s", err)
		}
	}
	{ // Get staged object from staging buffer
		objectMeta := api.ObjectMeta{Name: customers[1].Name}
		_, err := stagecl.BookstoreV1().Customer().Get(ctx, &objectMeta)
		if err == nil {
			t.Fatalf("Get of Order from staged buffer succeeded")
		}
	}
	{ // try to stage object to nonexistent buffer
		_, err := stagecl.BookstoreV1().Customer().Create(ctx, &customers[0])
		if err == nil {
			t.Fatalf("Create of Order into non-existent staging buffer succeeded")
		}
	}

}

func TestRestWatchers(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	// REST Client
	restcl, err := apiclient.NewRestAPIClient("http://localhost:" + tinfo.apigwport)
	if err != nil {
		t.Fatalf("cannot create REST client")
	}
	defer restcl.Close()
	// create logged in context
	ctx, err = NewLoggedInContext(ctx, "http://localhost:"+tinfo.apigwport, tinfo.userCred)
	AssertOk(t, err, "cannot create logged in context")

	var watch1Mutex sync.Mutex
	var watcher1Events, watcher1ExpEvents []kvstore.WatchEvent
	var watch2Mutex sync.Mutex
	var watcher2Events, watcher2ExpEvents []kvstore.WatchEvent
	var watch3Mutex sync.Mutex
	var watcher3Events, watcher3ExpEvents []kvstore.WatchEvent

	bl, err := restcl.BookstoreV1().Order().List(ctx, &api.ListWatchOptions{})
	AssertOk(t, err, "error getting list of objects")
	for _, v := range bl {
		meta := &api.ObjectMeta{Name: v.GetName()}
		_, err = restcl.BookstoreV1().Order().Delete(ctx, meta)
		AssertOk(t, err, fmt.Sprintf("error deleting object[%v](%s)", meta, err))
	}
	cnt, err := strconv.ParseInt(expvar.Get("api.cache.watchers").String(), 10, 32)
	if err != nil {
		t.Fatalf("parsing watch count failed (%s)", err)
	}
	waitWatch := make(chan error)
	go func() {
		t.Logf("Starting Watchers")
		watcher1, err := restcl.BookstoreV1().Order().Watch(ctx, &api.ListWatchOptions{})
		if err != nil {
			t.Fatalf("could not establish wathcer (%s)", err)
		}
		watcher2, err := restcl.BookstoreV1().Order().Watch(ctx, &api.ListWatchOptions{FieldSelector: "Spec.Order.Quantity=30"})
		if err != nil {
			t.Fatalf("could not establish wathcer (%s)", err)
		}
		watcher3, err := restcl.BookstoreV1().Order().Watch(ctx, &api.ListWatchOptions{FieldSelector: "Spec.Order.Quantity=20"})
		if err != nil {
			t.Fatalf("could not establish wathcer (%s)", err)
		}
		active := true
		t.Logf("Done starting Watchers")
		close(waitWatch)
		for active {
			select {
			case ev, ok := <-watcher1.EventChan():
				t.Logf("ts[%s] Publisher received event [%v]", time.Now(), ok)
				if ok {
					t.Logf("  event [%+v]", *ev)
					watch1Mutex.Lock()
					watcher1Events = append(watcher1Events, *ev)
					watch1Mutex.Unlock()
				} else {
					t.Logf("publisher watcher closed")
					active = false
				}
			case ev, ok := <-watcher2.EventChan():
				t.Logf("ts[%s] Publisher received event [%v]", time.Now(), ok)
				if ok {
					t.Logf("  event [%+v]", *ev)
					watch2Mutex.Lock()
					watcher2Events = append(watcher2Events, *ev)
					watch2Mutex.Unlock()
				} else {
					t.Logf("publisher watcher closed")
					active = false
				}
			case ev, ok := <-watcher3.EventChan():
				t.Logf("ts[%s] Publisher received event [%v]", time.Now(), ok)
				if ok {
					t.Logf("  event [%+v]", *ev)
					watch3Mutex.Lock()
					watcher3Events = append(watcher3Events, *ev)
					watch3Mutex.Unlock()
				} else {
					t.Logf("publisher watcher closed")
					active = false
				}
			}
		}
	}()
	<-waitWatch
	order := bookstore.Order{
		ObjectMeta: api.ObjectMeta{
			Name: "tesForPre-commithookToGenerateNewOrdeId",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Order",
		},
		Spec: bookstore.OrderSpec{
			Id: "order-1",
			Order: []*bookstore.OrderItem{
				{
					ISBNId:   "XXXX",
					Quantity: 30,
				},
			},
		},
	}
	{ // Create
		ret, err := restcl.BookstoreV1().Order().Create(ctx, &order)
		if err != nil {
			t.Fatalf("failed to create order object (%s)", err)
		}
		order.Name = ret.Name
		watcher1ExpEvents = addToWatchList(&watcher1ExpEvents, ret, kvstore.Created)
		watcher2ExpEvents = addToWatchList(&watcher2ExpEvents, ret, kvstore.Created)
	}
	{ // Update
		order.Spec.Order[0].Quantity = 20
		ret, err := restcl.BookstoreV1().Order().Update(ctx, &order)
		if err != nil {
			t.Fatalf("failed to update order object (%s)", err)
		}
		watcher1ExpEvents = addToWatchList(&watcher1ExpEvents, ret, kvstore.Updated)
		watcher3ExpEvents = addToWatchList(&watcher3ExpEvents, ret, kvstore.Updated)
	}
	{ // Delete
		ret, err := restcl.BookstoreV1().Order().Delete(ctx, &order.ObjectMeta)
		if err != nil {
			t.Fatalf("failed to delete order object (%s)", err)
		}
		watcher1ExpEvents = addToWatchList(&watcher1ExpEvents, ret, kvstore.Deleted)
		watcher3ExpEvents = addToWatchList(&watcher3ExpEvents, ret, kvstore.Deleted)
	}

	AssertEventually(t,
		func() (bool, interface{}) {
			defer watch1Mutex.Unlock()
			watch1Mutex.Lock()
			return len(watcher1ExpEvents) == len(watcher1Events), nil
		},
		fmt.Sprintf("failed to receive all watch1 events[%v/%v]", len(watcher1ExpEvents), len(watcher1Events)),
		"10ms",
		"3s")
	AssertEventually(t,
		func() (bool, interface{}) {
			defer watch1Mutex.Unlock()
			watch1Mutex.Lock()
			return len(watcher2ExpEvents) == len(watcher2Events), nil
		},
		fmt.Sprintf("failed to receive all watch2 events[%v/%v]", len(watcher2ExpEvents), len(watcher2Events)),
		"10ms",
		"3s")
	AssertEventually(t,
		func() (bool, interface{}) {
			defer watch1Mutex.Unlock()
			watch1Mutex.Lock()
			return len(watcher3ExpEvents) == len(watcher3Events), nil
		},
		fmt.Sprintf("failed to receive all watch3 events[%v/%v]", len(watcher3ExpEvents), len(watcher3Events)),
		"10ms",
		"3s")
	cancel()
	nwcnt := int64(0)
	AssertEventually(t, func() (bool, interface{}) {
		nwcnt, err = strconv.ParseInt(expvar.Get("api.cache.watchers").String(), 10, 32)
		if err != nil {
			t.Fatalf("parsing watch count failed (%s)", err)
		}
		// This assumes that the tests are not run in parallel.
		return nwcnt == cnt, nil
	}, fmt.Sprintf("watchers did not close [exp/got][%v/%v]", cnt, nwcnt), "100ms", "3s")

}
