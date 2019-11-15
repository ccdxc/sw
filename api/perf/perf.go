package main

import (
	"context"
	"flag"
	"fmt"
	"strings"
	"sync"
	"syscall"
	"time"

	"github.com/pensando/sw/api/generated/workload"

	"golang.org/x/crypto/ssh/terminal"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/workfarm"
)

type workCtx struct {
	numObjs int
	clients []apiclient.Services
	objs    []security.App
	wlObj   workload.Workload
	lctx    []context.Context
}

func work(ctx context.Context, id, iter int, userCtx interface{}) error {
	wctx := userCtx.(*workCtx)
	cl := wctx.clients[id%len(wctx.clients)]
	lctx := wctx.lctx[id%len(wctx.clients)]
	nctx, cancel := context.WithCancel(lctx)
	defer cancel()
	obj := wctx.objs[id%len(wctx.objs)]
	rdCh := make(chan error)
	go func() {
		_, err := cl.SecurityV1().App().Update(nctx, &obj)
		if err != nil {
			fmt.Printf("[%v]got error [%v][%v] (%s)\n", iter, obj.Tenant, obj.Name, err)
		}
		rdCh <- err
	}()
	var err error
	select {
	case <-ctx.Done():
		cancel()
		err = fmt.Errorf("context cancelled")
	case err = <-rdCh:
	}
	return err
}

func wlWork(ctx context.Context, id, iter int, userCtx interface{}) error {
	wctx := userCtx.(*workCtx)
	cl := wctx.clients[id%len(wctx.clients)]
	lctx := wctx.lctx[id%len(wctx.clients)]
	nctx, cancel := context.WithCancel(lctx)
	defer cancel()
	obj := wctx.wlObj
	obj.Name = fmt.Sprintf("workloadTest-%d", iter)
	obj.Spec.Interfaces[0].MACAddress = fmt.Sprintf("0000.0000.%04x", iter)
	obj.Spec.Interfaces[0].MicroSegVlan = uint32(iter%4000) + 1
	obj.Spec.Interfaces[0].ExternalVlan = 10

	rdCh := make(chan error)
	go func() {
		_, err := cl.WorkloadV1().Workload().Create(nctx, &obj)
		if err != nil {
			fmt.Printf("[%v]got error [%v][%v] (%s) [%+v]\n", iter, obj.Tenant, obj.Name, err, obj)
		}
		rdCh <- err
	}()
	var err error
	select {
	case <-ctx.Done():
		cancel()
		err = fmt.Errorf("context cancelled")
	case err = <-rdCh:
	}
	return err
}

type watcherCtx struct {
	sync.Mutex
	watchCount map[int]map[string]int
	clients    []apiclient.Services
}

func watcher(ctx context.Context, id, iter int, userCtx interface{}) error {
	wctx := userCtx.(*watcherCtx)
	reslts := make(map[string]int)
	cl := wctx.clients[id%len(wctx.clients)]
	opts := api.ListWatchOptions{}
	opts.Tenant = globals.DefaultTenant
	watcher, err := cl.NetworkV1().Network().Watch(ctx, &opts)
	defer watcher.Stop()
	if err != nil {
		return err
	}
	for {
		select {
		case ev, ok := <-watcher.EventChan():
			if !ok {
				return fmt.Errorf("watch returned error")
			}
			reslts[string(ev.Type)] = reslts[string(ev.Type)] + 1
		case <-ctx.Done():
			wctx.Lock()
			wctx.watchCount[id] = reslts
			wctx.Unlock()
			return nil
		}
	}
}

func main() {
	apigws := flag.String("gw", "", "comma seperated list of apigw urls")
	rate := flag.Int("rate", 0, "reqs per second, 0 (default) is blast at max possible")
	workers := flag.Int("workers", 10, "max number of workers")
	clients := flag.Int("clients", 10, "number of clients in client pool")
	iters := flag.Int("iters", 10000, "total iterations to run")
	maxTime := flag.String("timeout", "10m", "maximum time allowed for the test")
	numObjs := flag.Int("objs", 100, "number of objects to work on")
	uname := flag.String("user", "admin", "venice username")
	password := flag.String("passwd", "", "password to authenticate to venice")
	wlCreate := flag.Bool("wl", false, "create workload objects")
	delWl := flag.Bool("dwl", false, "Delete existing workloads")
	flag.Parse()

	gws := strings.Split(*apigws, ",")
	if len(gws) == 0 {
		fmt.Printf("No gateways provided\n")
		return
	}
	if *clients < 1 {
		fmt.Printf("number of clients should be greater than 1")
		return
	}
	if *workers < 1 {
		fmt.Printf("number of workers should be greater than 1")
		return
	}
	if *iters < 1 {
		fmt.Printf("number of workers should be greater than 1")
		return
	}

	if *delWl && *wlCreate {
		fmt.Printf("confusing params del and create!")
		return
	}

	maxDuration, err := time.ParseDuration(*maxTime)
	if err != nil {
		fmt.Printf("could not parse duration (%s)\n", err)
		return
	}
	indx := 0
	wctx := &workCtx{
		numObjs: *numObjs,
	}
	if *password == "" {
		fmt.Printf("Enter password: ")
		paswdBytes, err := terminal.ReadPassword(int(syscall.Stdin))
		if err != nil {
			fmt.Printf("error reading password (%s)\n", err)
			return
		}
		*password = string(paswdBytes)
		if *password == "" {
			fmt.Printf("password required for authentication\n")
			return
		}
	}
	userCred := &auth.PasswordCredential{
		Username: *uname,
		Password: *password,
		Tenant:   globals.DefaultTenant,
	}

	fmt.Printf("Starting test with paramters\n APIGw \t: %v\n Workers\t:%d\n Clients\t: %d\n Iterations\t: %d\n Objects \t: %d\n", gws, *workers, *clients, *iters, *numObjs)
	fmt.Printf("=== Setting up %d clients\n", *clients)
	ctx, cancel := context.WithCancel(context.Background())
	clientList := make([]apiclient.Services, 0)
	ctxList := make([]context.Context, 0)
	for i := 0; i < *clients; i++ {
		cl, err := apiclient.NewRestAPIClient(gws[indx%len(gws)])
		if err != nil {
			fmt.Printf("failed to create client to [%v](%s)", gws[indx%len(gws)], err)
			cancel()
			return
		}
		lctx, err := testutils.NewLoggedInContext(ctx, gws[indx%len(gws)], userCred)
		if err != nil {
			fmt.Printf("could not login (%s)", err)
			cancel()
			return
		}
		indx++
		defer cl.Close()
		clientList = append(clientList, cl)
		ctxList = append(ctxList, lctx)
	}
	wctx.clients = clientList
	wctx.lctx = ctxList
	if *delWl {
	}
	// Create test apps in default tenant
	fmt.Printf("=== Creating startup objects [%d]\n", *numObjs)
	if !*wlCreate {
		app := security.App{}
		app.Defaults("v1")
		app.Spec.ProtoPorts = append(app.Spec.ProtoPorts, security.ProtoPort{Protocol: "tcp", Ports: "1000"})
		app.Tenant = globals.DefaultTenant

		for i := 0; i < *numObjs; i++ {
			app.Name = fmt.Sprintf("test-app-%v", i)
			wctx.objs = append(wctx.objs, app)
			if _, err := wctx.clients[0].SecurityV1().App().Get(ctxList[0], &app.ObjectMeta); err != nil {
				_, err = wctx.clients[0].SecurityV1().App().Create(ctxList[0], &app)
				if err != nil {
					fmt.Printf("failed to create object [%v](%s)", app.Name, err)
				}
			}
		}
	} else {
		wctx.wlObj = workload.Workload{
			TypeMeta:   api.TypeMeta{Kind: "Workload"},
			ObjectMeta: api.ObjectMeta{Name: "workload-test", Tenant: "default"},
			Spec: workload.WorkloadSpec{
				HostName: "naples1-host",
				Interfaces: []workload.WorkloadIntfSpec{
					{
						MACAddress:   "0000.0000.0000",
						MicroSegVlan: 1,
					},
				},
			},
		}
		wlist, err := wctx.clients[0].WorkloadV1().Workload().List(ctxList[0], &api.ListWatchOptions{})
		if err != nil {
			fmt.Printf("failed to get existing worlload list (%s)", err)
			return
		}

		for _, wl := range wlist {
			if strings.HasPrefix(wl.Name, "workloadTest-") {
				wctx.clients[0].WorkloadV1().Workload().Delete(ctxList[0], &wl.ObjectMeta)
			}
		}
	}

	fmt.Printf("=== Starting run\n")
	farm := workfarm.New(*workers, time.Second, wlWork)
	ch, err := farm.Run(ctx, *iters, *rate, maxDuration, wctx)
	if err != nil {
		fmt.Printf("failed to start work (%s)\n", err)
	}
	rslts := <-ch
	cancel()
	fmt.Printf("=====> results <======\n%v\n", rslts.String())
}
