// client is a example REST client using the generated client bindings.
package main

import (
	"context"
	"flag"
	"log"
	"reflect"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/bookstore"
)

type respOrder struct {
	V   bookstore.Order
	Err error
}

func validateObject(expected, result interface{}) bool {
	exp := reflect.Indirect(reflect.ValueOf(expected)).FieldByName("Spec").Interface()
	res := reflect.Indirect(reflect.ValueOf(result)).FieldByName("Spec").Interface()

	if !reflect.DeepEqual(exp, res) {
		log.Printf("Values are %s[%+v] %s[%+v]", reflect.TypeOf(exp), exp, reflect.TypeOf(res), res)
		return false
	}
	return true
}

func main() {
	var (
		instance = flag.String("gwaddr", "localhost:9000", "API gateway to connect to")
	)
	flag.Parse()

	restcl, err := apiclient.NewRestAPIClient(*instance)
	if err != nil {
		log.Fatalf("cannot create REST client")
	}

	ctx := context.Background()
	var order1, order2 bookstore.Order
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
	}

	{ // ---  POST of the object via REST --- //
		retorder, err := restcl.BookstoreV1().Order().Create(ctx, &order1)
		if err != nil {
			log.Fatalf("Create of Order failed (%s)", err)
		}
		if !reflect.DeepEqual(retorder.Spec, order1.Spec) {
			log.Fatalf("Added Order object does not match \n\t[%+v]\n\t[%+v]", order1.Spec, retorder.Spec)
		}
	}

	{ // ---  POST second  object via REST --- //
		retorder, err := restcl.BookstoreV1().Order().Create(ctx, &order2)
		if err != nil {
			log.Fatalf("Create of Order failed (%s)", err)
		}
		if !reflect.DeepEqual(retorder.Spec, order2.Spec) {
			log.Fatalf("Added Order object does not match \n\t[%+v]\n\t[%+v]", order1.Spec, retorder.Spec)
		}
	}

	{ // ---  Get  object via REST --- //
		objectMeta := api.ObjectMeta{Name: "order-2"}
		retorder, err := restcl.BookstoreV1().Order().Get(ctx, &objectMeta)
		if err != nil {
			log.Fatalf("failed to get object Order via REST (%s)", err)
		}
		if !validateObject(retorder, order2) {
			log.Fatalf("updated object [Add] does not match \n\t[%+v]\n\t[%+v]", retorder, order2)
		}
	}

	{ // ---  LIST objects via REST --- //
		opts := api.ListWatchOptions{}
		retlist, err := restcl.BookstoreV1().Order().List(ctx, &opts)
		if err != nil {
			log.Fatalf("List operation returned error (%s)", err)
		}
		if len(retlist) != 2 {
			log.Fatalf("List expecting [2] elements got [%d]", len(retlist))
		}
	}

	{ // ---  PUT objects via REST --- //
		order2.Name = "order-2"
		order2.Spec.Order[0].ISBNId = "XXYY"
		order2.Spec.Order[0].Quantity = 33
		retorder, err := restcl.BookstoreV1().Order().Update(ctx, &order2)
		if err != nil {
			log.Fatalf("failed to update object Order via REST (%s)", err)
		}
		if !validateObject(retorder, order2) {
			log.Fatalf("updated object [Update] does not match \n\t[%+v]\n\t[%+v]", retorder, order2)
		}
	}

	{ // ---  DELETE objects via REST --- //
		objectMeta := api.ObjectMeta{Name: "order-1"}
		retorder, err := restcl.BookstoreV1().Order().Delete(ctx, &objectMeta)
		if err != nil {
			log.Fatalf("failed to delete object Order via REST (%s)", err)
		}
		if !validateObject(retorder, order1) {
			log.Fatalf("updated object [Delete] does not match \n\t[%+v]\n\t[%+v]", retorder, order1)
		}
	}
}
