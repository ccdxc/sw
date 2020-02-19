package impl

import (
	"context"
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/bookstore"
	apiintf "github.com/pensando/sw/api/interfaces"
	apisrvmocks "github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/kvstore"
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
	msg := apisrvmocks.NewFakeMessage("test.testMsg1", "/test/path", false).WithKvGetter(
		func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			retval := bookstore.Order{}
			err2 := kvs.Get(ctx, key, &retval)
			return retval, err2
		})

	apisrvmocks.SetFakeMethodReqType(msg, method)
	service.AddMethod("Order", method)
	s := &bookstoreHooks{
		logger: l,
		svc:    service,
	}
	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}
	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore %s", err)
	}
	txn := kvs.NewTxn()
	err = kvs.Create(ctx, order.MakeKey(""), &order)
	if err != nil {
		t.Errorf("object creation should succeed in KV (%s)", err)
	}

	i, ok, err := s.createNeworderID(ctx, kvs, txn, order.MakeKey(""), apiintf.CreateOper, false, order)
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

	i, ok, err = s.createNeworderID(ctx, kvs, txn, order.MakeKey(""), apiintf.CreateOper, false, book)
	if ok {
		t.Errorf("hook expected to fail due to wrong type")
	}

	i, ok, err = s.createNeworderID(ctx, kvs, txn, order.MakeKey(""), apiintf.UpdateOper, false, order)
	if !ok {
		t.Errorf("expecting to succeed but got error(%s)", err)
	}

	order.Status.Status = "SHIPPED"
	err = kvs.Update(ctx, order.MakeKey(""), &order)
	if err != nil {
		t.Errorf("object updation should succeed in KV (%s)", err)
	}

	i, ok, err = s.createNeworderID(ctx, kvs, txn, order.MakeKey(""), apiintf.UpdateOper, false, order)
	if ok {
		t.Errorf("expecting to fail")
	}
	s.processDelBook(ctx, apiintf.DeleteOper, book, false)
	errs := s.validateOrder(order, "v1", false, false)
	if errs != nil {
		t.Errorf("expecting validation to pass")
	}
}

func TestActionFunction(t *testing.T) {
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
	service := apisrvmocks.NewFakeService()
	method := apisrvmocks.NewFakeMethod(true)
	msg := apisrvmocks.NewFakeMessage("test.testMsg1", "/test/path", false).WithKvGetter(
		func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			retval := bookstore.Order{}
			err2 := kvs.Get(ctx, key, &retval)
			return retval, err2
		})
	apisrvmocks.SetFakeMethodReqType(msg, method)
	service.AddMethod("Order", method)
	s := &bookstoreHooks{
		logger: l,
		svc:    service,
	}
	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}
	kvs, err := store.New(storecfg)
	if err != nil {
		t.Fatalf("unable to create kvstore %s", err)
	}
	txn := kvs.NewTxn()

	err = kvs.Create(ctx, order.MakeKey(""), &order)
	if err != nil {
		t.Errorf("object creation should succeed in KV (%s)", err)
	}
	actreq := bookstore.ApplyDiscountReq{}
	_, ok, err := s.processApplyDiscountAction(ctx, kvs, txn, order.MakeKey(""), apiintf.CreateOper, false, actreq)
	if err != nil || ok {
		t.Errorf("expecing no error and kvwrite to be false, got [ %s/%v]", err, ok)
	}
	actreq.Coupon = "TESTFAIL"
	_, ok, err = s.processApplyDiscountAction(ctx, kvs, txn, order.MakeKey(""), apiintf.CreateOper, false, actreq)
	if err == nil || ok {
		t.Errorf("expecing error and kvwrite to be false, got [ %s/%v]", err, ok)
	}
	actreq.Coupon = ""

	_, ok, err = s.processClearDiscountAction(ctx, kvs, txn, order.MakeKey(""), apiintf.CreateOper, false, actreq)
	if err != nil || ok {
		t.Errorf("expecing no error and kvwrite to be false, got [ %s/%v]", err, ok)
	}
	_, ok, err = s.processRestockAction(ctx, kvs, txn, order.MakeKey(""), apiintf.CreateOper, false, order)
	if err != nil || ok {
		t.Errorf("expecing no error and kvwrite to be false, got [ %s/%v]", err, ok)
	}
	_, ok, err = s.processAddOutageAction(ctx, kvs, txn, order.MakeKey(""), apiintf.CreateOper, false, order)
	if err != nil || ok {
		t.Errorf("expecing no error and kvwrite to be false, got [ %s/%v]", err, ok)
	}
}
