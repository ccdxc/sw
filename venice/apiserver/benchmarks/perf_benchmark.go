package main

import (
	"context"
	"flag"
	"fmt"
	"io/ioutil"
	"math/rand"
	"net"
	"os"
	"strings"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	hdr "github.com/codahale/hdrhistogram"
	"google.golang.org/grpc"
	"google.golang.org/grpc/grpclog"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/venice/apiserver"
	rec "github.com/pensando/sw/venice/apiserver/benchmarks/utils"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/trace"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks"
)

type result struct {
	err     error
	elapsed time.Duration
}

type testResult struct {
	hist  *hdr.Histogram
	start time.Time
	end   time.Time
	count int
}

type tInfo struct {
	l             log.Logger
	scheme        *runtime.Scheme
	apicl         apiclient.Services
	apiclPool     []apiclient.Services
	workQ         chan func(context.Context, int) error
	doneQ         chan result
	closeWatchCh  chan error
	wg            sync.WaitGroup
	apiserverAddr string
	curhist       *testResult
	hist          map[string]*testResult
	count         int
}

var (
	tinfo          tInfo
	etcdcluster    = []string{"http://192.168.69.63:22379"}
	orderSetup     = false
	oper           = "update" // get or update
	totalReqs      uint64
	timeouts       uint64
	watcherrors    uint64
	printQuants    = []float64{10, 50, 75, 90, 99, 99.99}
	watcherCount   = 1000
	reqCount       = 100000
	workersCount   = 500
	objCount       = 5000
	clientPoolSize = 500
)

func account() {
	atomic.AddUint64(&totalReqs, uint64(tinfo.count))
}

func watcher(t *tInfo, id int) {
	ctx, cancel := context.WithCancel(context.Background())
	defer tinfo.wg.Done()
	defer cancel()
	opts := api.ListWatchOptions{}
	w, err := tinfo.apiclPool[id%clientPoolSize].BookstoreV1().Order().Watch(ctx, &opts)
	if err != nil {
		panic(fmt.Sprintf("Unable to watch %s", err))
	}
	for {
		select {
		case <-tinfo.closeWatchCh:
			cancel()
			return
		case _, ok := <-w.EventChan():
			if !ok {
				atomic.AddUint64(&watcherrors, 1)
				cancel()
				ctx, cancel = context.WithCancel(context.Background())
				w, err = tinfo.apiclPool[id%clientPoolSize].BookstoreV1().Order().Watch(ctx, &opts)
			}
		}
	}
}

func worker(t *tInfo, id int) {
	ctx, cancel := context.WithCancel(context.Background())
	defer tinfo.wg.Done()
	rch := make(chan error)
	for {
		work, ok := <-t.workQ
		if !ok {
			cancel()
			return
		}
		start := time.Now()
		go func() {
			rch <- work(ctx, id)
		}()
		var e error
		select {
		case e = <-rch:
		case <-time.After(1 * time.Second):
			cancel()
			atomic.AddUint64(&timeouts, 1)
			ctx, cancel = context.WithCancel(context.Background())
		}
		done := result{err: e, elapsed: time.Since(start)}
		t.doneQ <- done
	}
}

func startWorkers() {
	tinfo.workQ = make(chan func(context.Context, int) error, workersCount)
	tinfo.doneQ = make(chan result, workersCount)
	for i := 0; i < workersCount; i++ {
		tinfo.wg.Add(1)
		go worker(&tinfo, i)
	}
}

func setupAPIServer(kvtype string, cluster []string, pool int) {
	apiserverAddress := ":0"
	srvconfig := apiserver.Config{
		GrpcServerPort: apiserverAddress,
		DebugMode:      false,
		DevMode:        false,
		Logger:         tinfo.l,
		Version:        "v1",
		Scheme:         tinfo.scheme,
		Kvstore: store.Config{
			Type:    kvtype,
			Codec:   runtime.NewJSONCodec(tinfo.scheme),
			Servers: cluster,
		},
		KVPoolSize: pool,
	}
	trace.Init("ApiServer")
	trace.DisableOpenTrace()
	srv := apiserverpkg.MustGetAPIServer()
	go srv.Run(srvconfig)
	srv.WaitRunning()
	addr, err := srv.GetAddr()
	if err != nil {
		os.Exit(-1)
	}
	_, port, err := net.SplitHostPort(addr)
	if err != nil {
		panic(fmt.Sprintf("could not resolve Apiserver address"))
	}
	tinfo.apiserverAddr = "localhost" + ":" + port

	for i := 0; i < clientPoolSize; i++ {
		apicl, err := cache.NewGrpcUpstream(tinfo.apiserverAddr, tinfo.l, cache.WithSetDevMode(false))
		if err != nil {
			panic(fmt.Sprintf("cannot create grpc client [%s] (%s)", tinfo.apiserverAddr, err))
		}
		tinfo.apiclPool = append(tinfo.apiclPool, apicl)
	}
	if !orderSetup {
		orderSetup = true
		book := bookstore.Book{
			ObjectMeta: api.ObjectMeta{
				Name: "overridden",
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
		for i := 0; i < objCount; i++ {
			book.Name = fmt.Sprintf("Volume-%d", i)
			tinfo.apiclPool[0].BookstoreV1().Book().Create(context.TODO(), &book)
			// ignore error for now.
		}
	}
}

func shutdownAPIServer() {
	srv := apiserverpkg.MustGetAPIServer()
	close(tinfo.workQ)
	//close(tinfo.closeWatchCh)
	tinfo.wg.Wait()
	srv.Stop()
	for i := range tinfo.apiclPool {
		tinfo.apiclPool[i].Close()
	}
	tinfo.apiclPool = nil
}

func activateWorkers(fn func(context.Context, int) error) {
	var wg sync.WaitGroup
	wg.Add(1)
	tinfo.l.Infof("activating workers")
	go func() {
		tinfo.l.Infof("sending to  workers")
		for i := 0; i < tinfo.count; i++ {
			select {
			case tinfo.workQ <- fn:

			case <-time.After(1 * time.Second):
				panic("Timeout sending work")
			}
		}
		tinfo.l.Infof("finished workers")
		wg.Done()
	}()

	count := 0
	for count < tinfo.count {
		rslt := <-tinfo.doneQ
		tinfo.curhist.hist.RecordValue(rslt.elapsed.Nanoseconds())
		count++
	}
	wg.Wait()
}

func setupMemKvBackedAPIServer(pool int, b *testing.B) {
	setupAPIServer(store.KVStoreTypeMemkv, []string{""}, pool)
}

func setupEtcdBackedAPIServer(cluster []string, pool int) {
	setupAPIServer(store.KVStoreTypeEtcd, cluster, pool)
}

func runBenchmark(name string) {
	if _, ok := tinfo.hist[name]; !ok {
		tinfo.hist[name] = &testResult{
			hist:  hdr.New(0, 10000000000, 4),
			count: tinfo.count,
		}
	}
	tinfo.curhist = tinfo.hist[name]

	objectMeta := api.ObjectMeta{Name: "order-2"}
	r := rand.New(rand.NewSource(int64(time.Now().Nanosecond())))
	var fn func(context.Context, int) error
	if oper == "get" {
		fn = func(ctx context.Context, id int) error {
			objectMeta.Name = fmt.Sprintf("Volume-%d", (r.Int31() % int32(objCount)))
			_, err := tinfo.apiclPool[id%clientPoolSize].BookstoreV1().Book().Get(ctx, &objectMeta)
			return err
		}
	} else {
		fn = func(ctx context.Context, id int) error {
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
			book.Name = fmt.Sprintf("Volume-%d", id%clientPoolSize)
			_, err := tinfo.apiclPool[id%clientPoolSize].BookstoreV1().Book().Update(ctx, &book)
			return err
		}
	}
	tinfo.curhist.start = time.Now()
	activateWorkers(fn)
	tinfo.curhist.end = time.Now()
	account()
}

func runTest(clntCount int) {
	tinfo.count = reqCount
	setupEtcdBackedAPIServer(etcdcluster, clntCount)
	startWorkers()
	runBenchmark(fmt.Sprintf("etcd%d", clntCount))
	shutdownAPIServer()
}

func runList() {
	tinfo.count = reqCount
	setupEtcdBackedAPIServer(etcdcluster, 1)
	opt := api.ListWatchOptions{}
	r, err := tinfo.apiclPool[0].BookstoreV1().Book().List(context.TODO(), &opt)
	if err != nil {
		fmt.Printf("List returned error (%s)\n", err)
	} else {
		fmt.Printf("List succeeded got %d items\n", len(r))
	}
	shutdownAPIServer()
}

func main() {
	operp := flag.String("oper", "get", "operation to benchmark")
	reqCountp := flag.Int("req-count", 100000, "number of requests")
	workersCountP := flag.Int("workers", 500, "number of workers to spin")
	objCountp := flag.Int("obj-count", 5000, "number of objects")
	kvstore := flag.String("kvdest", "localhost:2379", "Comma seperated list of etcd servers")
	clntPoolSizep := flag.Int("api-clients", 500, "number of api server clients for workers to use")
	list := flag.Bool("list", false, "Run list operation only")
	flag.Parse()

	reqCount = *reqCountp
	workersCount = *workersCountP
	objCount = *objCountp
	clientPoolSize = *clntPoolSizep
	etcdcluster = strings.Split(*kvstore, ",")

	grpc.EnableTracing = false
	trace.DisableOpenTrace()
	l := log.WithContext("module", "CrudOpsTest")
	l.SetOutput(ioutil.Discard)
	c := log.GetDefaultConfig("test")
	c.LogToStdout = false
	c.LogToFile = false
	log.SetConfig(c)
	tinfo.l = l
	tinfo.scheme = runtime.NewScheme()
	tinfo.workQ = make(chan func(context.Context, int) error, workersCount)
	tinfo.doneQ = make(chan result, workersCount)
	tinfo.hist = make(map[string]*testResult)
	oper = *operp
	grpclog.SetLogger(l)
	if *list {
		runList()
	} else {
		runTest(1)
		runTest(1000)
	}
	fmt.Printf("Total requests: %d timeouts: %d\n", totalReqs, timeouts)
	rec.PrintAll()
	for k, v := range tinfo.hist {
		perreq := (v.end.Sub(v.start).Nanoseconds()) / int64(v.count)
		fmt.Printf("%s  : With %d Clients and %d poolsize\n", k, workersCount, clientPoolSize)
		fmt.Printf(" Reqs/Sec: %d\n Mean (%.3f)msec\n Min (%.3f)msec\n Max (%.3f)msec\n StdDev(%.3f)\n",
			time.Second.Nanoseconds()/perreq,
			float64(v.hist.Mean())/float64(time.Millisecond),
			float64(v.hist.Min())/float64(time.Millisecond),
			float64(v.hist.Max())/float64(time.Millisecond),
			float64(v.hist.ValueAtQuantile(99))/float64(time.Millisecond))
		for _, q := range printQuants {
			fmt.Printf("	[%.2f] : %.3fmsec\n", q, float64(v.hist.ValueAtQuantile(q))/float64(time.Millisecond))
		}
	}
}
