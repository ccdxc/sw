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
	"strings"
	"time"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/testutils"

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

func delApps(instance string) {
	restcl, err := apiclient.NewRestAPIClient(instance)
	if err != nil {
		log.Fatalf("cannot create REST client")
	}
	defer restcl.Close()
	userCred := &auth.PasswordCredential{
		Username: "test",
		Password: "Pensando0$",
		Tenant:   globals.DefaultTenant,
	}
	ctx, err := testutils.NewLoggedInContext(context.Background(), instance, userCred)
	if err != nil {
		fmt.Printf("could not login (%s)", err)
		return
	}

	applist, err := restcl.SecurityV1().App().List(ctx, &api.ListWatchOptions{})
	if err != nil {
		fmt.Printf("list returned error (%s)", err)
		return
	}
	count := 0
	for _, a := range applist {
		if strings.Contains(a.Name, "scaleSnapshotApp") {
			_, err = restcl.SecurityV1().App().Delete(ctx, &a.ObjectMeta)
			if err != nil {
				fmt.Printf("could not delete app [%v](%s)", a.Name, err)
			}
			count++
		}
	}
	fmt.Printf("deleted %d apps", count)
}

func testAppSGPolLoop(instance string, count int) {
	restcl, err := apiclient.NewRestAPIClient(instance)
	if err != nil {
		log.Fatalf("cannot create REST client")
	}
	defer restcl.Close()
	userCred := &auth.PasswordCredential{
		Username: "test",
		Password: "Pensando0$",
		Tenant:   globals.DefaultTenant,
	}
	ctx, err := testutils.NewLoggedInContext(context.Background(), instance, userCred)
	if err != nil {
		fmt.Printf("could not login (%s)", err)
		return
	}

	// Create initial objects
	app1 := security.App{
		TypeMeta: api.TypeMeta{
			Kind: "App",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "testapp1",
			Tenant: globals.DefaultTenant,
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{Protocol: "tcp", Ports: "2110"},
			},
		},
	}
	app2 := security.App{
		TypeMeta: api.TypeMeta{
			Kind: "App",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "testapp2",
			Tenant: globals.DefaultTenant,
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{Protocol: "tcp", Ports: "2111"},
			},
		},
	}
	app3 := security.App{
		TypeMeta: api.TypeMeta{
			Kind: "App",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "testapp3",
			Tenant: globals.DefaultTenant,
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{Protocol: "tcp", Ports: "2112"},
			},
		},
	}

	sgpol := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{
			Kind: "NetworkSecurityPolicy",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "testpol1",
			Tenant: globals.DefaultTenant,
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,

			Rules: []security.SGRule{
				{
					Action:          "PERMIT",
					FromIPAddresses: []string{"0.0.0.0"},
					ToIPAddresses:   []string{"0.0.0.0"},
					Apps:            []string{"testapp1", "testapp2", "testapp3"},
				},
			},
		},
	}
	restcl.SecurityV1().App().Delete(ctx, &app1.ObjectMeta)
	restcl.SecurityV1().App().Delete(ctx, &app2.ObjectMeta)
	restcl.SecurityV1().App().Delete(ctx, &app3.ObjectMeta)

	restcl.SecurityV1().NetworkSecurityPolicy().Delete(ctx, &sgpol.ObjectMeta)

	// Enter loop
	for i := 0; i < count; i++ {
		_, err = restcl.SecurityV1().App().Create(ctx, &app1)
		if err != nil {
			fmt.Printf("failed to create App1 [%d] (%s)\n", i, err)
			return
		}
		_, err = restcl.SecurityV1().App().Create(ctx, &app2)
		if err != nil {
			fmt.Printf("failed to create App2 [%d] (%s)\n", i, err)
			return
		}
		_, err = restcl.SecurityV1().App().Create(ctx, &app3)
		if err != nil {
			fmt.Printf("failed to create App3 [%d] (%s)\n", i, err)
			return
		}
		_, err = restcl.SecurityV1().NetworkSecurityPolicy().Create(ctx, &sgpol)
		if err != nil {
			fmt.Printf("failed to create SGPol [%d] (%s)\n", i, err)
			return
		}
		_, err = restcl.SecurityV1().NetworkSecurityPolicy().Delete(ctx, &sgpol.ObjectMeta)
		if err != nil {
			fmt.Printf("failed to Delete SGPol [%d] (%s)\n", i, err)
			return
		}
		_, err = restcl.SecurityV1().App().Delete(ctx, &app1.ObjectMeta)
		if err != nil {
			fmt.Printf("failed to Delete App1 [%d] (%s)\n", i, err)
			return
		}
		_, err = restcl.SecurityV1().App().Delete(ctx, &app2.ObjectMeta)
		if err != nil {
			fmt.Printf("failed to Delete App2 [%d] (%s)\n", i, err)
			return
		}
		_, err = restcl.SecurityV1().App().Delete(ctx, &app3.ObjectMeta)
		if err != nil {
			fmt.Printf("failed to Delete App3 [%d] (%s)\n", i, err)
			return
		}
	}
	fmt.Printf("completed %d iterations\n", count)
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
				Type:   network.NetworkType_Bridged.String(),
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
				Type:   network.NetworkType_Bridged.String(),
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
				Type:   network.NetworkType_Bridged.String(),
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
		instance    = flag.String("gwaddr", "localhost:19000", "API gateway to connect to")
		testStaging = flag.Bool("staging", false, "Use Api server staging")
		testScaleSg = flag.Bool("scaleSg", false, "test scaled SG policy post")
		testWebSock = flag.Bool("ws", false, "test websocket watch")
		testCD      = flag.Bool("tl", false, "test Create delete loop")
		tcount      = flag.Int("count", 100, "iteration count")
		daps        = flag.Bool("dapps", true, "delapps")
	)
	flag.Parse()

	if *daps {
		delApps(*instance)
	}
	if *testStaging {
		testStagingFn(*instance)
		return
	}

	if *testCD {
		testAppSGPolLoop(*instance, *tcount)
		return
	}

	ctx := context.Background()
	restcl, err := apiclient.NewRestAPIClient(*instance)
	if err != nil {
		log.Fatalf("cannot create REST client")
	}

	if *testWebSock {
		watcher, err := restcl.BookstoreV1().Order().Watch(ctx, &api.ListWatchOptions{})
		if err != nil {
			log.Fatalf("failed to create a watcher (%s)", err)
		}
		for {
			select {
			case ev, ok := <-watcher.EventChan():
				if ok {
					fmt.Printf("received Order Event [ %+v]\n", ev)
				} else {
					fmt.Printf("channel closed!!")
					return
				}
			case <-ctx.Done():
				return
			}
		}
	} else if *testScaleSg {
		// Create SG Policy.
		jsonFile, err := ioutil.ReadFile("/tmp/70k_sg_policy.json")
		if err != nil {
			fmt.Printf("open file failed: %s\n", err)
			return
		}
		sgp := &security.NetworkSecurityPolicy{}
		err = json.Unmarshal(jsonFile, sgp)
		if err != nil {
			fmt.Printf("failed to parse security policy (%s)", err)
			return
		}
		fmt.Printf("Number of rule in security policy is %d\n", len(sgp.Spec.Rules))

		start := time.Now()
		_, err = restcl.SecurityV1().NetworkSecurityPolicy().Create(ctx, sgp)
		if err != nil {
			fmt.Printf("failed to create securing policy (%s)", err)
			return
		}
		elapsed := time.Since(start)
		fmt.Printf("Put took %s\n", elapsed)
		return
	}
	var orders = []bookstore.Order{
		{
			ObjectMeta: api.ObjectMeta{
				Name: "POST",
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
		},
		{
			ObjectMeta: api.ObjectMeta{
				Name: "testfoPOST",
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
		},
	}
	var names []string
	{ // ---  POST of the object via REST --- //
		retorder, err := restcl.BookstoreV1().Order().Create(ctx, &orders[0])
		if err != nil {
			log.Fatalf("Create of Order failed (%s)", err)
		}
		orders[0].Spec.Id = retorder.Name
		if !reflect.DeepEqual(retorder.Spec, orders[0].Spec) {
			log.Fatalf("Added Order object does not match \n\t[%+v]\n\t[%+v]", orders[0].Spec, retorder.Spec)
		}
		names = append(names, retorder.Name)
	}

	{ // ---  POST second  object via REST --- //
		retorder, err := restcl.BookstoreV1().Order().Create(ctx, &orders[1])
		if err != nil {
			log.Fatalf("Create of Order failed (%s)", err)
		}
		orders[1].Spec.Id = retorder.Name
		if !reflect.DeepEqual(retorder.Spec, orders[1].Spec) {
			log.Fatalf("Added Order object does not match \n\t[%+v]\n\t[%+v]", orders[1].Spec, retorder.Spec)
		}
		names = append(names, retorder.Name)
	}

	{ // ---  Get  object via REST --- //
		objectMeta := api.ObjectMeta{Name: names[1]}
		retorder, err := restcl.BookstoreV1().Order().Get(ctx, &objectMeta)
		if err != nil {
			log.Fatalf("failed to get object Order via REST (%s)", err)
		}
		if !validateObject(retorder, orders[1]) {
			log.Fatalf("updated object [Add] does not match \n\t[%+v]\n\t[%+v]", retorder, orders[1])
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
	fmt.Printf("press enter to update object")
	bufio.NewReader(os.Stdin).ReadBytes('\n')
	{ // ---  PUT objects via REST --- //
		orders[1].Name = names[1]
		orders[1].Spec.Order[0].ISBNId = "XXYY"
		orders[1].Spec.Order[0].Quantity = 3
		retorder, err := restcl.BookstoreV1().Order().Update(ctx, &orders[1])
		if err != nil {
			log.Fatalf("failed to update object Order via REST (%s)", err)
		}
		if !validateObject(retorder, orders[1]) {
			log.Fatalf("updated object [Update] does not match \n\t[%+v]\n\t[%+v]", retorder, orders[1])
		}
		fmt.Printf("update object is [%+v]\n", retorder)
	}

	fmt.Printf("press enter to delete objects")
	bufio.NewReader(os.Stdin).ReadBytes('\n')

	{ // ---  DELETE objects via REST --- //
		for k, n := range names {
			objectMeta := api.ObjectMeta{Name: n}
			retorder, err := restcl.BookstoreV1().Order().Delete(ctx, &objectMeta)
			if err != nil {
				log.Fatalf("failed to delete object Order via REST (%s)", err)
			}

			if !validateObject(retorder, orders[k]) {
				log.Fatalf("updated object [Delete] does not match \n\t[%+v]\n\t[%+v]", retorder, orders[k])
			}
		}
	}
}
