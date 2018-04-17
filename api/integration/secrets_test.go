package integration

import (
	"context"
	"fmt"
	"math/rand"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var numWatchEvents uint32

func checkStoredObject(ctx context.Context, t *testing.T, refObject *bookstore.Customer) {
	storedObj := bookstore.Customer{}
	err := tinfo.cache.Get(ctx, refObject.MakeKey("bookstore"), &storedObj)
	AssertOk(t, err, fmt.Sprintf("Error reading object from KvStore:  %v", err))
	txStoredObj, err := bookstore.StorageCustomerTransformer.TransformFromStorage(ctx, storedObj)
	AssertOk(t, err, "Error transforming object from storage")
	if !validateObjectSpec(txStoredObj, refObject) {
		t.Fatalf("Stored object failed validation. Want: %+v, have: %+v", refObject, txStoredObj)
	}
	// Make sure the transformer actually does something
	Assert(t, !validateObjectSpec(storedObj, txStoredObj), "Transformer has no effect!")
}

func doWatch(t *testing.T, watcher kvstore.Watcher, validateObjectFn func(obj interface{}) bool, waitWatch chan bool, wg *sync.WaitGroup) {
	close(waitWatch)
	active := true
	for active {
		select {
		case ev, ok := <-watcher.EventChan():
			if ok {
				if !validateObjectFn(ev.Object) {
					t.Fatalf("Watch event failed validation: [%+v]", *ev)
				}
				atomic.AddUint32(&numWatchEvents, 1)
			} else {
				t.Logf("customer watcher closed")
				active = false
			}
		}
	}
	wg.Done()
}

func TestSecretsTransformer(t *testing.T) {
	apiserverAddr := "localhost" + ":" + tinfo.apiserverport
	ctx := context.Background()

	// gRPC client emulating Npm -- should see any secret in plaintext
	npmClient, err := client.NewGrpcUpstream(globals.Npm, apiserverAddr, tinfo.l)
	AssertOk(t, err, "cannot create Npm grpc client")

	// gRPC client emulating ApiGw -- should not see any secret either via direct access or watch
	apiGwClient, err := client.NewGrpcUpstream(globals.APIGw, apiserverAddr, tinfo.l)
	AssertOk(t, err, "cannot create ApiGw grpc client")

	// REST Client -- should not see any secret
	restClient, err := apiclient.NewRestAPIClient("http://localhost:" + tinfo.apigwport)
	AssertOk(t, err, "cannot create REST client")

	fullCustomer := bookstore.Customer{
		ObjectMeta: api.ObjectMeta{
			Name: "FullCustomer",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Customer",
		},
		Spec: bookstore.CustomerSpec{
			Address:           "123 My Way",
			CreditCardNumbers: []string{"0123456789", "9876543210"},
			Password:          []byte{0xDE, 0xAD, 0xBE, 0xEF},
			PasswordRecoveryInfo: bookstore.CustomerPersonalInfo{
				SSN:              "555-67-1234",
				MotherMaidenName: "Mamma",
			},
		},
	}

	scrubbedCustomer := bookstore.Customer{
		ObjectMeta: api.ObjectMeta{
			Name: "ScrubbedCustomer",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Customer",
		},
		Spec: bookstore.CustomerSpec{
			Address: "123 My Way",
		},
	}

	var wg sync.WaitGroup
	wctx, cancel := context.WithCancel(ctx)
	listOpts := api.ListWatchOptions{}

	wg.Add(1)
	apiGwWaitWatch := make(chan bool)
	apiGwWatcher, err := apiGwClient.BookstoreV1().Customer().Watch(wctx, &listOpts)
	if err != nil {
		t.Fatalf("Failed to start watch (%s)\n", err)
	}
	var apiGwValidateFn = func(obj interface{}) bool {
		return validateObjectSpec(obj, scrubbedCustomer)
	}
	go doWatch(t, apiGwWatcher, apiGwValidateFn, apiGwWaitWatch, &wg)

	wg.Add(1)
	npmWaitWatch := make(chan bool)
	npmWatcher, err := npmClient.BookstoreV1().Customer().Watch(wctx, &listOpts)
	if err != nil {
		t.Fatalf("Failed to start watch (%s)\n", err)
	}
	var npmValidateFn = func(obj interface{}) bool {
		return validateObjectSpec(obj, fullCustomer)
	}
	go doWatch(t, npmWatcher, npmValidateFn, npmWaitWatch, &wg)

	// Wait for watches to be established
	<-apiGwWaitWatch
	<-npmWaitWatch

	type clientParams struct {
		client apiclient.Services
		refObj *bookstore.Customer
	}

	testClients := []clientParams{
		clientParams{
			client: npmClient,
			refObj: &fullCustomer,
		},
		clientParams{
			client: apiGwClient,
			refObj: &scrubbedCustomer,
		},
		clientParams{
			client: restClient,
			refObj: &scrubbedCustomer,
		},
	}

	seed := rand.NewSource(time.Now().UnixNano())
	t.Logf("Random seed is %v", seed)
	r := rand.New(seed)

	for _, i := range r.Perm(len(testClients)) {
		c := testClients[i]
		fullCustomer.ObjectMeta.Name = fmt.Sprintf("FullCustomer-%d", i)
		ret, err := c.client.BookstoreV1().Customer().Create(ctx, &fullCustomer)
		AssertOk(t, err, fmt.Sprintf("Error creating object with client %d: %v", i, err))
		if !validateObjectSpec(ret, c.refObj) {
			t.Fatalf("Returned object failed validation. Client: %d, want: %+v, have: %+v", i, c.refObj, ret)
		}
		// check raw KVStore content
		checkStoredObject(ctx, t, &fullCustomer)
	}

	for _, i := range r.Perm(len(testClients)) {
		c := testClients[i]
		objs, err := c.client.BookstoreV1().Customer().List(ctx, &listOpts)
		AssertOk(t, err, fmt.Sprintf("Error listing customer with client %d: %v", i, err))
		t.Logf("List content: [%+v] ", objs)

		Assert(t, len(objs) == len(testClients), fmt.Sprintf("List result contains unexpected number of object: %+v", objs))
		for _, obj := range objs {
			if !validateObjectSpec(obj, c.refObj) {
				t.Fatalf("Returned object failed validation. Client: %d, want: %+v, have: %+v", i, c.refObj, obj)
			}
		}
	}

	for _, i := range r.Perm(len(testClients)) {
		c := testClients[i]
		fullCustomer.ObjectMeta.Name = fmt.Sprintf("FullCustomer-%d", i)
		ret, err := c.client.BookstoreV1().Customer().Update(ctx, &fullCustomer)
		AssertOk(t, err, fmt.Sprintf("Error updating customer with client %d: %v", i, err))
		if !validateObjectSpec(ret, c.refObj) {
			t.Fatalf("Returned object failed validation. Client: %d, want: %+v, have: %+v", i, c.refObj, ret)
		}
		// check raw KVStore content
		checkStoredObject(ctx, t, &fullCustomer)
	}

	for _, i := range r.Perm(len(testClients)) {
		c := testClients[i]
		meta := api.ObjectMeta{Name: fmt.Sprintf("FullCustomer-%d", i)}
		ret, err := c.client.BookstoreV1().Customer().Get(ctx, &meta)
		AssertOk(t, err, fmt.Sprintf("Error getting object with client %d: %v", i, err))
		if !validateObjectSpec(ret, c.refObj) {
			t.Fatalf("Returned object failed validation. Client: %d, want: %+v, have: %+v", i, c.refObj, ret)
		}
	}

	for _, i := range r.Perm(len(testClients)) {
		c := testClients[i]
		meta := api.ObjectMeta{Name: fmt.Sprintf("FullCustomer-%d", i)}
		ret, err := c.client.BookstoreV1().Customer().Delete(ctx, &meta)
		AssertOk(t, err, fmt.Sprintf("Error deleting object with client %d: %v", i, err))
		if !validateObjectSpec(ret, c.refObj) {
			t.Fatalf("Deleted object failed validation. Client: %d, want: %+v, have: %+v", i, c.refObj, ret)
		}
	}

	expNumWatchEvents := uint32(2 * 3 * len(testClients)) // 2 watchers * 3 operations (create, update, delete) * num clients
	AssertEventually(t, func() (bool, interface{}) { return numWatchEvents == expNumWatchEvents, nil },
		fmt.Sprintf("unexpected number of watch events, want: %d, have: %d", expNumWatchEvents, numWatchEvents),
		"10ms",
		"9s")

	cancel()
	wg.Wait()
}
