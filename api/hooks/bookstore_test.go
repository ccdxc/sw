package impl

import (
	"context"
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/venice/apiserver"
	apisrvmocks "github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

func TestCreateNewOrderId(t *testing.T) {
	ctx := context.Background()
	logConfig := log.GetDefaultConfig("TestBookstoreHooks")
	l := log.GetNewLogger(logConfig)
	order := bookstore.Order{
		ObjectMeta: api.ObjectMeta{
			Name:            "Bad Order Name",
			ResourceVersion: "10",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Order",
		},
		Spec: bookstore.OrderSpec{
			Id: "Bad Order Id",
		},
		Status: bookstore.OrderStatus{
			Status: "JunkValue",
		},
	}
	book := bookstore.Book{
		ObjectMeta: api.ObjectMeta{
			Name: fmt.Sprintf("Volume-%d", 0),
		},
		TypeMeta: api.TypeMeta{
			Kind: "Book",
		},
		Spec: bookstore.BookSpec{
			ISBNId:   "0000000000",
			Author:   "noname",
			Category: "Fiction",
		},
	}

	service := apisrvmocks.NewFakeService()
	method := apisrvmocks.NewFakeMethod(true)
	msg := apisrvmocks.NewFakeMessage("/test/path", false)
	apisrvmocks.SetFakeMessageKvObj(order, msg)
	apisrvmocks.SetFakeMethodReqType(msg, method)
	service.AddMethod("test", method)
	s := &bookstoreHooks{
		logger: l,
		svc:    service,
	}
	storecfg := store.Config{
		Type:  store.KVStoreTypeMemkv,
		Codec: runtime.NewJSONCodec(runtime.NewScheme()),
	}
	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore %s", err)
	}
	txn := kvs.NewTxn()

	i, ok, err := s.createNeworderID(ctx, kvs, txn, "/test/key", apiserver.CreateOper, order)
	if !ok {
		t.Errorf("hook failed, expecting pass")
	}
	ret := i.(bookstore.Order)
	if ret.Name != fmt.Sprintf("order-%d", s.orderID) {
		t.Errorf("expecting name [%s] to be changed, found [%s]", fmt.Sprintf("order-%d", s.orderID), ret.Name)
	}

	if ret.Status.Status != "PROCESSING" {
		t.Errorf("unexpected Status field [%s]", ret.Status.Status)
	}

	i, ok, err = s.createNeworderID(ctx, kvs, txn, "/test/key", apiserver.CreateOper, book)
	if ok {
		t.Errorf("hook expected to fail due to wrong type")
	}

	i, ok, err = s.createNeworderID(ctx, kvs, txn, "/test/key", apiserver.UpdateOper, order)
	if !ok {
		t.Errorf("expecting to succeed")
	}
	order.Status.Status = "SHIPPED"
	apisrvmocks.SetFakeMessageKvObj(order, msg)
	i, ok, err = s.createNeworderID(ctx, kvs, txn, "/test/key", apiserver.UpdateOper, order)
	if ok {
		t.Errorf("expecting to fail")
	}
	s.processDelBook(ctx, apiserver.DeleteOper, book)
	err = s.validateOrder(order, "v1", false)
	if err != nil {
		t.Errorf("expecting validation to pass")
	}
}
