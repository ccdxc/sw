// client is a example REST client using the generated client bindings.
package main

import (
	"bufio"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"reflect"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/staging"
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

func testStagingFn(instance string) {
	var (
		bufName    = "TestBuffer"
		tenantName = "default"
	)
	restcl, err := apiclient.NewRestAPIClient(instance)
	if err != nil {
		log.Fatalf("cannot create REST client")
	}
	defer restcl.Close()

	stagecl, err := apiclient.NewStagedRestAPIClient(instance, bufName)
	if err != nil {
		log.Fatalf("cannot create Staged REST client")
	}
	defer stagecl.Close()

	ctx := context.Background()

	{ // Cleanup existing
		objMeta := api.ObjectMeta{Tenant: tenantName, Name: bufName}
		restcl.StagingV1().Buffer().Delete(ctx, &objMeta)
	}
	fmt.Printf("creating Staging Buffer\n")
	{ // Create a buffer
		buf := staging.Buffer{}
		buf.Name = bufName
		buf.Tenant = tenantName
		_, err := restcl.StagingV1().Buffer().Create(ctx, &buf)
		if err != nil {
			fmt.Printf("*** Failed to create Staging Buffer(%s)\n", err)
		}
	}
	fmt.Printf("creating Staged Objects\n")
	{ // Create a staged object
		netw := network.Network{
			TypeMeta: api.TypeMeta{
				Kind:       "tenant",
				APIVersion: "v1",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant2",
				Name:   "network1",
			},
			Spec: network.NetworkSpec{
				Type:   "vlan",
				VlanID: 10,
			},
		}
		_, err := stagecl.NetworkV1().Network().Create(ctx, &netw)
		if err != nil {
			log.Fatalf("Could not create Staged network object(%s)", err)
		}
	}

	{ // Create a staged object
		netw := network.Network{
			TypeMeta: api.TypeMeta{
				Kind:       "tenant",
				APIVersion: "v1",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant2",
				Name:   "network2",
			},
			Spec: network.NetworkSpec{
				Type:   "vlan",
				VlanID: 10,
			},
		}
		_, err := stagecl.NetworkV1().Network().Create(ctx, &netw)
		if err != nil {
			log.Fatalf("Could not create Staged network object(%s)", err)
		}
	}
	{ // Create a staged object
		netw := network.Network{
			TypeMeta: api.TypeMeta{
				Kind:       "tenant",
				APIVersion: "v1",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant2",
				Name:   "network3",
			},
			Spec: network.NetworkSpec{
				Type:   "vlan",
				VlanID: 10,
			},
		}
		_, err := stagecl.NetworkV1().Network().Create(ctx, &netw)
		if err != nil {
			log.Fatalf("Could not create Staged network object(%s)", err)
		}
	}

	{ // Get a buffer
		objMeta := api.ObjectMeta{Tenant: tenantName, Name: bufName}
		buf, err := restcl.StagingV1().Buffer().Get(ctx, &objMeta)
		if err != nil {
			fmt.Printf("*** Failed to get Staging Buffer(%s)\n", err)
		}
		fmt.Printf(" Received buffer is [%+v]\n", buf)
	}

	{ // List buffers
		opts := api.ListWatchOptions{}
		opts.Tenant = tenantName
		bufs, err := restcl.StagingV1().Buffer().List(ctx, &opts)
		if err != nil {
			fmt.Printf("*** Failed to list Staging Buffer(%s)\n", err)
		}
		fmt.Printf(" Received List is [%+v]\n", bufs)
	}

	fmt.Printf("press enter to continue to commit")
	bufio.NewReader(os.Stdin).ReadBytes('\n')
	{ // Commit the buffer
		action := &staging.CommitAction{}
		action.Name = bufName
		action.Tenant = tenantName
		resp, err := restcl.StagingV1().Buffer().Commit(ctx, action)
		if err != nil {
			fmt.Printf("*** Failed to commit Staging Buffer(%s)\n", err)
		}
		fmt.Printf("Commit response is %+v\n", resp)
	}
	fmt.Printf("press enter to continue to stage delete")
	bufio.NewReader(os.Stdin).ReadBytes('\n')
	{ // Delete a staged object
		meta := api.ObjectMeta{
			Tenant: "tenant2",
			Name:   "network2",
		}
		_, err := stagecl.NetworkV1().Network().Delete(ctx, &meta)
		if err != nil {
			log.Fatalf("Could not delete Staged network object(%s)", err)
		}
	}
	fmt.Printf("press enter to continue to commit")
	bufio.NewReader(os.Stdin).ReadBytes('\n')
	{ // Commit the buffer
		action := &staging.CommitAction{}
		action.Name = bufName
		action.Tenant = tenantName
		resp, err := restcl.StagingV1().Buffer().Commit(ctx, action)
		if err != nil {
			fmt.Printf("*** Failed to commit Staging Buffer(%s)\n", err)
		}
		fmt.Printf("Commit response is %+v\n", resp)
	}
	fmt.Printf("press enter to continue to delete buffer")
	bufio.NewReader(os.Stdin).ReadBytes('\n')
	{ // Delete a buffer
		objMeta := api.ObjectMeta{Tenant: tenantName, Name: bufName}
		buf, err := restcl.StagingV1().Buffer().Delete(ctx, &objMeta)
		if err != nil {
			fmt.Printf("*** Failed to delete Staging Buffer(%s)\n", err)
		}
		fmt.Printf(" Delete received buffer is [%+v]\n", buf)
	}
}
func main() {
	var (
		instance    = flag.String("gwaddr", "localhost:9000", "API gateway to connect to")
		testStaging = flag.Bool("staging", false, "Use Api server staging")
		testScaleSg = flag.Bool("scaleSg", false, "test scaled SG policy post")
	)
	flag.Parse()

	if *testStaging {
		testStagingFn(*instance)
		return
	}
	ctx := context.Background()
	restcl, err := apiclient.NewRestAPIClient(*instance)
	if err != nil {
		log.Fatalf("cannot create REST client")
	}

	if *testScaleSg {
		// Create SG Policy.
		jsonFile, err := ioutil.ReadFile("/tmp/70k_sg_policy.json")
		if err != nil {
			fmt.Printf("open file failed: %s\n", err)
			return
		}
		sgp := &security.SGPolicy{}
		err = json.Unmarshal(jsonFile, sgp)
		if err != nil {
			fmt.Printf("failed to parse security policy (%s)", err)
			return
		}
		fmt.Printf("Number of rule in security policy is %d\n", len(sgp.Spec.Rules))

		start := time.Now()
		_, err = restcl.SecurityV1().SGPolicy().Create(ctx, sgp)
		if err != nil {
			fmt.Printf("failed to create securing policy (%s)", err)
			return
		}
		elapsed := time.Since(start)
		fmt.Printf("Put took %s\n", elapsed)
		return
	}
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
