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

	goruntime "runtime"
	"runtime/pprof"

	hdr "github.com/codahale/hdrhistogram"
	gogotypes "github.com/gogo/protobuf/types"
	"google.golang.org/grpc"
	"google.golang.org/grpc/grpclog"

	rl "github.com/juju/ratelimit"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	rec "github.com/pensando/sw/venice/utils/histogram"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/trace"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apiserver"
)

type result struct {
	err     error
	elapsed time.Duration
}

type testResult struct {
	hist  *hdr.Histogram
	whist *hdr.Histogram
	start time.Time
	end   time.Time
	count int
}

type tInfo struct {
	l              log.Logger
	scheme         *runtime.Scheme
	apicl          apiclient.Services
	apiclPool      []apiclient.Services
	workQ          chan func(context.Context, int) error
	doneQ          chan result
	closeWatchCh   chan error
	wg             sync.WaitGroup
	watchwg        sync.WaitGroup
	watchReadyWg   sync.WaitGroup
	apiserverAddr  string
	curhist        *testResult
	hist           map[string]*testResult
	count          int
	useCache       bool
	enableWatchers int
	bucket         *rl.Bucket
	profStart      chan error
	memtrace       bool
}

var (
	tinfo          tInfo
	etcdcluster    = []string{"http://192.168.69.63:22379"}
	orderSetup     = true
	oper           = "update" // get or update
	totalReqs      uint64
	totalWatchEv   uint64
	activeWatchers int64
	timeouts       uint64
	reqSends       uint64
	respErrs       uint64
	respErrMap     map[string]bool
	watcherrors    uint64
	printQuants    = []float64{10, 50, 75, 90, 99, 99.99}
	watcherCount   = 1000
	reqCount       = 100000
	workersCount   = 500
	objCount       = 5000
	clientPoolSize = 500
)

var histmap = []string{
	"kvstore.Update",
	"store.Set",
	"store.Set.Lock",
	"store.GetOp",
	"store.SetOp",
	"KVCBGetQs",
	"store.CB",
	"CB.GetQs",
	"CB.Enqueue",
	"watch.Enqueue.Lock",
	"watch.Enqueue.Insert",
	"safelist.Lock",
	"safelist.PushBack",
	"watch.Enqueue.Notify",
	"cache.Update",
	"kvstore.WatchLatency",
	"WatchWorkerPreCache",
	"watch.DequeueLatency",
	"watch.SendEvent",
	"kvstore.WatchWorker",
}

func account() {
	atomic.AddUint64(&totalReqs, uint64(tinfo.count))
}

func watcher(t *tInfo, id int) {
	ctx, cancel := context.WithCancel(context.Background())
	atomic.AddInt64(&activeWatchers, 1)
	defer atomic.AddInt64(&activeWatchers, -1)
	defer tinfo.watchwg.Done()
	defer cancel()
	opts := api.ListWatchOptions{}
	w, err := tinfo.apiclPool[id%clientPoolSize].BookstoreV1().Book().Watch(ctx, &opts)
	if err != nil {
		panic(fmt.Sprintf("Unable to watch %s", err))
	}
	// Ready to watch - signal
	tinfo.watchReadyWg.Done()
	for {
		select {
		case <-tinfo.closeWatchCh:
			cancel()
			return
		case o, ok := <-w.EventChan():
			if !ok {
				atomic.AddUint64(&watcherrors, 1)
				cancel()
				ctx, cancel = context.WithCancel(context.Background())
				w, err = tinfo.apiclPool[id%clientPoolSize].BookstoreV1().Book().Watch(ctx, &opts)
			} else {
				obj := o.Object
				book := obj.(*bookstore.Book)
				tm, _ := book.Spec.UpdateTimestamp.Time()
				tinfo.curhist.whist.RecordValue(time.Since(tm).Nanoseconds())
				atomic.AddUint64(&totalWatchEv, uint64(1))
				if o.Type == kvstore.Updated && book.Spec.Terminate {
					cancel()
					return
				}
			}
		}
	}
}

func startWatchers() {
	for i := 0; i < tinfo.enableWatchers; i++ {
		tinfo.watchwg.Add(1)
		tinfo.watchReadyWg.Add(1)
		go watcher(&tinfo, i)
	}
	tinfo.watchReadyWg.Wait()
}

func deactivateWatchers() error {
	book := bookstore.Book{
		ObjectMeta: api.ObjectMeta{
			Name: fmt.Sprintf("Volume-%d", 0),
		},
		TypeMeta: api.TypeMeta{
			Kind: "Book",
		},
		Spec: bookstore.BookSpec{
			ISBNId:    "0000000000",
			Author:    "noname",
			Category:  "Fiction",
			Terminate: true,
		},
	}
	ts, _ := gogotypes.TimestampProto(time.Now())
	book.Spec.UpdateTimestamp = &api.Timestamp{Timestamp: *ts}
	book.Name = fmt.Sprintf("Volume-0")
	for {
		_, err := tinfo.apiclPool[0].BookstoreV1().Book().Update(context.Background(), &book)
		if err != nil {
			tinfo.l.Infof("Terminate update for watchers failed (%s)\n", err)
			atomic.AddUint64(&timeouts, 1)
		} else {
			break
		}
	}
	fmt.Printf("Deactivating Watchers Current Watch Stats: %d/%d/%d active:%d timeouts: %d\n", atomic.LoadUint64(&totalReqs),
		atomic.LoadUint64(&totalWatchEv), atomic.LoadUint64(&watcherrors),
		atomic.LoadInt64(&activeWatchers), atomic.LoadUint64(&timeouts))
	waitch := make(chan error)
	go func() {
		tinfo.watchwg.Wait()
		waitch <- nil
	}()
	wait := true
	for wait {
		select {
		case <-time.After(1 * time.Second):
			fmt.Printf("Deactiving Watchers Current Watch Stats: %d/%d/%d active:%d timeouts: %d\n", atomic.LoadUint64(&totalReqs),
				atomic.LoadUint64(&totalWatchEv), atomic.LoadUint64(&watcherrors),
				atomic.LoadInt64(&activeWatchers), atomic.LoadUint64(&timeouts))
		case <-waitch:
			wait = false
		}
	}
	return nil
}

func worker(t *tInfo, id int) {
	ctx, cancel := context.WithCancel(context.Background())
	defer tinfo.wg.Done()
	rch := make(chan error)
	var wg sync.WaitGroup
	for {
		work, ok := <-t.workQ
		if !ok {
			cancel()
			wg.Wait()
			return
		}
		var start time.Time
		wg.Add(1)
		go func() {
			start = time.Now()
			select {
			case rch <- work(ctx, id):
			default:
			}
			atomic.AddUint64(&reqSends, 1)
			wg.Done()
		}()
		var e error
		select {
		case e = <-rch:
			if e != nil {
				atomic.AddUint64(&respErrs, 1)
			}
		case <-time.After(3 * time.Second):
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
	if tinfo.useCache {
		cachecfg := cache.Config{
			Config: store.Config{
				Type:    kvtype,
				Codec:   runtime.NewJSONCodec(tinfo.scheme),
				Servers: cluster,
			},
			NumKvClients: pool,
			Logger:       tinfo.l,
		}
		cache, err := cache.CreateNewCache(cachecfg)
		if err != nil {
			panic("failed to create cache")
		}
		srvconfig.CacheStore = cache
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
		apicl, err := client.NewGrpcUpstream("perf_benchmark", tinfo.apiserverAddr, tinfo.l, client.WithSetDevMode(false))
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
		var wg sync.WaitGroup
		crFunc := func(i int, b bookstore.Book) {
			defer wg.Done()
			b.Name = fmt.Sprintf("Volume-%d", i)
			ts, _ := gogotypes.TimestampProto(time.Now())
			b.Spec.UpdateTimestamp = &api.Timestamp{Timestamp: *ts}
			tinfo.apiclPool[0].BookstoreV1().Book().Create(context.TODO(), &b)
			// ignore error for now.
		}
		for i := 0; i < objCount; i++ {
			book.Name = fmt.Sprintf("Volume-%d", i)
			wg.Add(1)
			go crFunc(i, book)
		}
		wg.Wait()
		fmt.Printf("Done Creating Objects\n")
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
	start := time.Now()
	wg.Add(1)
	tinfo.l.Infof("activating workers")
	fmt.Printf("activating workers\n")
	go func() {
		tinfo.l.Infof("sending to  workers")
		for i := 0; i < tinfo.count; i++ {
			if tinfo.bucket != nil {
				tinfo.bucket.Wait(1)
			}
			select {
			case tinfo.workQ <- fn:
			case <-time.After(2 * time.Second):
				panic("Timeout sending work")
			}
		}
		tinfo.l.Infof("finished workers")
		wg.Done()
	}()

	count := 0
	for count < tinfo.count {
		rslt := <-tinfo.doneQ
		if rslt.err == nil {
			tinfo.curhist.hist.RecordValue(rslt.elapsed.Nanoseconds())
		}
		count++
	}
	fmt.Printf("received from all workers %d Took: %dmsecs\n", count, time.Since(start).Nanoseconds()/int64(time.Millisecond))
	wg.Wait()
}

func setupMemKvBackedAPIServer(pool int, b *testing.B) {
	setupAPIServer(store.KVStoreTypeMemkv, []string{""}, pool)
}

func setupEtcdBackedAPIServer(cluster []string, pool int) {
	setupAPIServer(store.KVStoreTypeEtcd, cluster, pool)
}

func runBenchmark(name string) {
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
			ts, _ := gogotypes.TimestampProto(time.Now())
			book.Spec.UpdateTimestamp = &api.Timestamp{Timestamp: *ts}
			book.Name = fmt.Sprintf("Volume-%d", id%clientPoolSize)
			_, err := tinfo.apiclPool[id%clientPoolSize].BookstoreV1().Book().Update(ctx, &book)
			return err
		}
	}
	if tinfo.memtrace {
		// Start profiling memory
		tinfo.profStart <- nil
		<-tinfo.profStart
	}
	tinfo.curhist.start = time.Now()
	activateWorkers(fn)
	tinfo.curhist.end = time.Now()
	deactivateWatchers()
	account()
}

func runTest(clntCount int) {
	name := fmt.Sprintf("etcd%d", clntCount)
	tinfo.count = reqCount
	if _, ok := tinfo.hist[name]; !ok {
		tinfo.hist[name] = &testResult{
			hist:  hdr.New(0, 10000000000, 4),
			whist: hdr.New(0, 10000000000, 4),
			count: tinfo.count,
		}
	}
	tinfo.curhist = tinfo.hist[name]
	setupEtcdBackedAPIServer(etcdcluster, clntCount)
	if tinfo.enableWatchers != 0 {
		startWatchers()
		fmt.Printf("Watchers Started\n")
	}
	respErrMap = make(map[string]bool)
	fmt.Printf("Starting Workers\n")
	startWorkers()
	fmt.Printf("Starting Test %s\n", name)
	runBenchmark(name)
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
	kvstore := flag.String("kvdest", "localhost:2379", "Comma separated list of etcd servers")
	clntPoolSizep := flag.Int("api-clients", 500, "number of api server clients for workers to use")
	wcount := flag.Int("watchers", 0, "Number of Watchers to start")
	list := flag.Bool("list", false, "Run list operation only")
	dsetup := flag.Bool("dsetup", false, "Setup datastore")
	cpuprofile := flag.String("cpuprof", "", "cpu profile output file")
	memprofile := flag.String("memprof", "", "mem profile output file")
	usecache := flag.Bool("usecache", true, "use cache between API server and KV store")
	lstdout := flag.Bool("lstdout", false, "enable logging to stdout")
	rlimit := flag.Int("rlimit", 0, "rate-limit req/sec")
	memtrace := flag.Bool("mtrace", false, "trace memory usage")
	flag.Parse()

	reqCount = *reqCountp
	workersCount = *workersCountP
	objCount = *objCountp
	clientPoolSize = *clntPoolSizep
	etcdcluster = strings.Split(*kvstore, ",")

	if *dsetup {
		orderSetup = false
	}
	if *cpuprofile != "" {
		f, err := os.Create(*cpuprofile)
		if err != nil {
			log.Fatalf("Unable to open cpu profile file %s", err)
		}
		if err := pprof.StartCPUProfile(f); err != nil {
			log.Fatalf("Unable to start cpu profiler  %s", err)
		}
		defer pprof.StopCPUProfile()
	}

	if *memprofile != "" {
		memprofFunc := func() {
			f, err := os.Create(*memprofile)
			if err != nil {
				log.Fatalf("Unable to open mem profile file %s", err)
			}
			goruntime.GC()
			if err := pprof.WriteHeapProfile(f); err != nil {
				log.Fatalf("Unable to start mem profiler  %s", err)
			}
			f.Close()
		}
		defer memprofFunc()
	}
	var memStats []goruntime.MemStats
	var profWg sync.WaitGroup
	stopProf := make(chan error)
	if *memtrace {
		profWg.Add(1)
		go func() {
			<-tinfo.profStart
			var m goruntime.MemStats
			goruntime.ReadMemStats(&m)
			memStats = append(memStats, m)
			close(tinfo.profStart)
			ticker := time.NewTicker(1 * time.Second)
			for {
				select {
				case <-stopProf:
					profWg.Done()
					return
				case <-ticker.C:
					goruntime.ReadMemStats(&m)
					memStats = append(memStats, m)
				}
			}
		}()
	}
	grpc.EnableTracing = false
	trace.DisableOpenTrace()
	l := log.WithContext("module", "CrudOpsTest")
	c := log.GetDefaultConfig("test")
	c.LogToStdout = false
	c.LogToFile = false
	if *dsetup {
		orderSetup = false
	}
	if *lstdout {
		c.LogToStdout = true
	}
	tinfo.enableWatchers = *wcount
	log.SetConfig(c)
	if c.LogToFile == false && c.LogToStdout == false {
		l.SetOutput(ioutil.Discard)
	}
	if *rlimit != 0 {
		tinfo.bucket = rl.NewBucketWithRate(float64(*rlimit), int64(workersCount))
	}
	tinfo.l = l
	tinfo.scheme = runtime.NewScheme()
	tinfo.workQ = make(chan func(context.Context, int) error, workersCount)
	tinfo.doneQ = make(chan result, workersCount)
	tinfo.hist = make(map[string]*testResult)
	tinfo.useCache = *usecache
	tinfo.profStart = make(chan error)
	tinfo.memtrace = *memtrace
	oper = *operp
	grpclog.SetLogger(l)
	if *list {
		runList()
	} else {
		// runTest(1)
		runTest(1000)
	}
	if *memtrace {
		close(stopProf)
		profWg.Wait()
	}
	fmt.Printf("Total requests: %d/%d timeouts: %d\n", totalReqs, reqSends, timeouts)
	fmt.Printf("response errors: %d len: %d\n", respErrs, len(respErrMap))
	for k := range respErrMap {
		fmt.Printf("errors: %s\n", k)
	}
	rec.PrintAll()
	for k, v := range tinfo.hist {
		perreq := (v.end.Sub(v.start).Nanoseconds()) / int64(v.count)
		fmt.Printf("%s  : With %d Clients and %d poolsize\n", k, workersCount, clientPoolSize)
		fmt.Printf(" Count: %d\n", v.hist.TotalCount())
		fmt.Printf(" Reqs/Sec: %d\n Mean (%.3f)msec\n Min (%.3f)msec\n Max (%.3f)msec\n StdDev(%.3f)\n",
			time.Second.Nanoseconds()/perreq,
			float64(v.hist.Mean())/float64(time.Millisecond),
			float64(v.hist.Min())/float64(time.Millisecond),
			float64(v.hist.Max())/float64(time.Millisecond),
			float64(v.hist.ValueAtQuantile(99))/float64(time.Millisecond))
		for _, q := range printQuants {
			fmt.Printf("	[%.2f] : %.3fmsec\n", q, float64(v.hist.ValueAtQuantile(q))/float64(time.Millisecond))
		}
		fmt.Printf(" Watch Statistics:\n")
		fmt.Printf(" Count: %d\n", v.whist.TotalCount())
		fmt.Printf(" Mean (%.3f)msec\n Min (%.3f)msec\n Max (%.3f)msec\n StdDev(%.3f)\n",
			float64(v.whist.Mean())/float64(time.Millisecond),
			float64(v.whist.Min())/float64(time.Millisecond),
			float64(v.whist.Max())/float64(time.Millisecond),
			float64(v.whist.ValueAtQuantile(99))/float64(time.Millisecond))
		for _, q := range printQuants {
			fmt.Printf("    [%.2f] : %.3fmsec\n", q, float64(v.whist.ValueAtQuantile(q))/float64(time.Millisecond))
		}
	}
	if *memtrace {
		fmt.Printf("Memory usage statistics\n")
		fmt.Printf("Alloc,TotalAllocs,Sys,Mallocs,Frees,HeapAlloc,HeapInuse,StackInuse\n")
		for i := 0; i < len(memStats); i++ {
			v := memStats[i]
			fmt.Printf("%d,%d,%d,%d,%d,%d,%d\n",
				v.Alloc, v.TotalAlloc,
				v.Mallocs, v.Frees,
				v.HeapAlloc, v.HeapInuse, v.StackInuse)
		}
	}
}
