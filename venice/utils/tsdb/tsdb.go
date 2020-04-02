package tsdb

import (
	"context"
	"fmt"
	"math"
	"net"
	"net/http"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/citadel/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	defaultRanges                  = 10
	defaultSendInterval            = 10 * time.Second
	defaultConnectionRetryInterval = 100 * time.Millisecond
	defaultNumPoints               = 150 // save points generated in 15 minutes (max. 5 instance)
)

var (
	maxMetricsPoints      = defaultNumPoints
	minCollectionInterval = 5 * time.Second
)

// global information is maintained per client during Init time
type globalInfo struct {
	opts          Opts                   // user options
	sync.Mutex                           // global lock
	wg            sync.WaitGroup         // waitgroup for threads
	rpcClient     *rpckit.RPCClient      // rpc connection to collector
	mc            metric.MetricApiClient // metric client object (to write points)
	context       context.Context        // global context (used for cancellation)
	objs          map[string]*iObj       // cache for various objs
	deletedObjs   []*iObj                // deleted list of objs
	httpServer    *http.Server           // local http server
	listener      net.Listener           // listener used by http server
	cancelFunc    context.CancelFunc     // cancel function to initiate internal cleanup
	sendErrors    uint64                 // send error stats
	numPoints     uint64                 // number of points sent to TSDB
	ignoredPoints uint64                 // number of points failed to add in TSDB
}

var global *globalInfo

// Opts define global options for tsdb package initialization
type Opts struct {
	ClientName              string             // name provided to collector
	ResolverClient          resolver.Interface // resolver for getting the whereabouts of collector
	Collector               string             // resolvable name of collector; system default is picked up if nil
	DBName                  string             // backend database name
	ConnectionRetryInterval time.Duration      // sleep period between connection retries
	SendInterval            time.Duration      // push interval, system default is picked up if zero
	LocalPort               int                // port to start local server on
	StartLocalServer        bool               // whether to start a local server or not
}

// Init initializes the tsdb package; must be called before any other tsdb apis can be used
func Init(ctx context.Context, opts *Opts) {
	if global != nil {
		global.opts.SendInterval = opts.SendInterval
		global.opts.ConnectionRetryInterval = opts.ConnectionRetryInterval
		return
	}
	global = &globalInfo{}
	global.opts = *opts
	if opts.SendInterval == 0 {
		global.opts.SendInterval = defaultSendInterval
	}
	if opts.ConnectionRetryInterval == 0 {
		global.opts.ConnectionRetryInterval = defaultConnectionRetryInterval
	}

	global.objs = make(map[string]*iObj)
	global.context, global.cancelFunc = context.WithCancel(ctx)

	global.wg.Add(1)
	go periodicTransmit()
	global.wg.Add(1)
	go collectionTimer()
	global.wg.Add(1)
	go startLocalRESTServer(global)
}

// IsInitialized returns whether tsdb has been initialized
func IsInitialized() bool {
	return global != nil
}

// Start starts tsdb export
func Start(resolverClient resolver.Interface) error {
	global.opts.ResolverClient = resolverClient
	return nil
}

// Cleanup pushes all pending metrics and frees up resources allocated to the tsdb client
func Cleanup() {
	if global == nil {
		return
	}
	stopLocalRESTServer()

	// cancel the context to stop any running go threads
	global.cancelFunc()
	global.wg.Wait() // Wait for startLocalRESTServer call to complete ; Also wait for periodic timer Thread to exit

	global = nil
}

// Obj represents a series within a measurement
type Obj interface {
	Delete()
	Counter(field string) api.Counter
	Gauge(field string) api.Gauge
	PrecisionGauge(field string) api.PrecisionGauge
	String(field string) api.String
	Bool(field string) api.Bool
	Histogram(field string) api.Histogram
	Summary(field string) api.Summary
	Points(points []*Point, ts time.Time) error
	AtomicBegin(ts time.Time)
	AtomicEnd()
	Push()
}

// ObjOpts specifies options for a specific obj
type ObjOpts struct {
	// local objects are not exported
	Local bool

	// minimum interval to collect the object metric
	CollectionInterval time.Duration
	// collectionTicks is the number of times metrics are collected for this object within on send interval
	collectionTicks int
}

// implementation of Obj interface
type iObj struct {
	sync.Mutex
	tableName       string
	opts            ObjOpts
	keys            map[string]string
	fields          map[string]interface{}
	ts              time.Time
	metricPoints    []*metric.MetricPoint
	atomic          bool
	dirty           bool
	collectionTicks int
}

// NewObj creates a metric object that can be used to record arbitrary keys/fields
func NewObj(tableName string, keys map[string]string, metrics interface{}, opts *ObjOpts) (Obj, error) {
	if global == nil {
		return nil, fmt.Errorf("failed to create %v, tsdb is not initialized", tableName)
	}
	if strings.HasPrefix(tableName, "obj-") {
		return nil, fmt.Errorf("Obj Name starting with 'obj-' is reserved for internal objs")
	}
	if opts != nil && opts.CollectionInterval > 0 && opts.CollectionInterval < minCollectionInterval {
		return nil, fmt.Errorf("collection interval %d must be more than minimum %d", opts.CollectionInterval, minCollectionInterval)
	}

	global.Lock()
	defer global.Unlock()

	// objName is uniquely determind from the set of keys
	// if the keys overlap, then object is considered existing
	// and an existing value is returned
	if len(keys) == 0 {
		keys = map[string]string{"name": tableName}
	}
	objName := getObjName(keys)

	obj, ok := global.objs[objName]
	if ok {
		return obj, nil
	}

	obj = &iObj{}
	obj.tableName = tableName
	obj.fields = make(map[string]interface{})
	obj.keys = keys

	// if metrics are provided during this object creation, fill the fields based on
	// supplied metrics structure
	if err := fillFields(obj, metrics); err != nil {
		return nil, err
	}

	if opts != nil {
		obj.opts = *opts
		obj.opts.collectionTicks = int(opts.CollectionInterval.Seconds() / minCollectionInterval.Seconds())
		obj.collectionTicks = obj.opts.collectionTicks
	}

	// every object is added to global object's hash based on object's name
	global.objs[objName] = obj

	return obj, nil
}

// NewVeniceObj creates a metric object for the specified venice object and its metrics
func NewVeniceObj(obj interface{}, metrics interface{}, opts *ObjOpts) (Obj, error) {
	if metrics == nil {
		return nil, fmt.Errorf("metrics object missing")
	}

	keys := make(map[string]string)
	tableName, err := getKeys(obj, keys)
	if err != nil {
		return nil, err
	}

	return NewObj(tableName, keys, metrics, opts)
}

// Delete cleans up resources associated with the obj
func (obj *iObj) Delete() {
	global.Lock()
	defer global.Unlock()
	global.deletedObjs = append(global.deletedObjs, obj)
	objName := getObjName(obj.keys)
	delete(global.objs, objName)
}

// AtomicBegin marks the start of atomic update on a set of metrics
func (obj *iObj) AtomicBegin(ts time.Time) {
	obj.Lock()
	defer obj.Unlock()
	if ts.IsZero() {
		ts = time.Now()
	}
	obj.ts = ts
	obj.atomic = true
}

// AtomicEnd marks the end of an atomic update
func (obj *iObj) AtomicEnd() {
	obj.Lock()
	defer obj.Unlock()
	obj.atomic = false
	obj.dirty = true
}

// Counter
type iCounter struct {
	name  string
	obj   *iObj
	value int64
}

// Add increments the counter by the specified value
func (c *iCounter) Add(inc int64) {
	obj := c.obj
	obj.Lock()
	defer obj.Unlock()

	c.value += inc
	obj.dirty = true
}

// Sub decrements the counter by the specified value
func (c *iCounter) Sub(inc int64) {
	obj := c.obj
	obj.Lock()
	defer obj.Unlock()

	c.value -= inc
	obj.dirty = true
}

// Set sets the counter value to a specified value
func (c *iCounter) Set(val int64) {
	obj := c.obj
	obj.Lock()
	defer obj.Unlock()

	c.value = val
	obj.dirty = true
}

// Add increments the counter by one
func (c *iCounter) Inc() {
	obj := c.obj
	obj.Lock()
	defer obj.Unlock()

	c.value++
	obj.dirty = true
}

// Dec decrements the counter by one
func (c *iCounter) Dec() {
	obj := c.obj
	obj.Lock()
	defer obj.Unlock()

	c.value--
	obj.dirty = true
}

// Counter function creates and returns a new counter metric
func (obj *iObj) Counter(name string) api.Counter {
	obj.Lock()
	defer obj.Unlock()

	c, ok := obj.fields[name].(*iCounter)
	if !ok {
		c = &iCounter{name: name, obj: obj}
		obj.fields[name] = c
	}
	return c
}

// Gauge - a bounded float value
type iGauge struct {
	name  string
	obj   *iObj
	value float64
}

// Set registers a value of a gauge
func (g *iGauge) Set(val float64) {
	obj := g.obj

	obj.Lock()
	defer obj.Unlock()

	g.value = val
	obj.dirty = true
}

// PrecisionGauge creates a gauge metric
func (obj *iObj) Gauge(name string) api.Gauge {
	obj.Lock()
	defer obj.Unlock()

	g, ok := obj.fields[name].(*iGauge)
	if !ok {
		g = &iGauge{name: name, obj: obj}
		obj.fields[name] = g
	}
	return g
}

// PrecisionGauge - is gauge allowing a point with a timestamp for every value
type iPrecisionGauge struct {
	name  string
	obj   *iObj
	value float64
}

// Set registers a value of a gauge
func (g *iPrecisionGauge) Set(val float64, ts time.Time) {
	obj := g.obj

	obj.Lock()
	defer obj.Unlock()

	if ts.IsZero() {
		ts = time.Now()
	}

	createNewMetricPoint(obj, ts)

	g.value = val
	if !obj.atomic {
		obj.ts = ts
	}
	obj.dirty = true
}

// PrecisionGauge creates a gauge metric
func (obj *iObj) PrecisionGauge(name string) api.PrecisionGauge {
	obj.Lock()
	defer obj.Unlock()

	g, ok := obj.fields[name].(*iPrecisionGauge)
	if !ok {
		g = &iPrecisionGauge{name: name, obj: obj}
		obj.fields[name] = g
	}
	return g
}

// Bool
type iBool struct {
	name  string
	obj   *iObj
	value bool
}

// Set registers a value of a bool
func (b *iBool) Set(val bool, ts time.Time) {
	obj := b.obj
	obj.Lock()
	defer obj.Unlock()

	// if the value hasn't change, avoid updating timestamp
	if b.value == val {
		return
	}

	if ts.IsZero() {
		ts = time.Now()
	}

	createNewMetricPoint(obj, ts)

	b.value = val
	if !obj.atomic {
		obj.ts = ts
	}
	obj.dirty = true
}

// Bool creates a boolean metric
func (obj *iObj) Bool(name string) api.Bool {
	obj.Lock()
	defer obj.Unlock()

	b, ok := obj.fields[name].(*iBool)
	if !ok {
		b = &iBool{name: name, obj: obj}
		obj.fields[name] = b
	}
	return b
}

// String
type iString struct {
	name  string
	obj   *iObj
	value string
}

// Set registers a value of a bool
func (s *iString) Set(val string, ts time.Time) {
	obj := s.obj
	obj.Lock()
	defer obj.Unlock()

	// if the value hasn't change, avoid updating timestamp
	if s.value == val {
		return
	}

	if ts.IsZero() {
		ts = time.Now()
	}

	createNewMetricPoint(obj, ts)

	s.value = val
	if !obj.atomic {
		obj.ts = ts
	}
	obj.dirty = true
}

// String creates a string metric
func (obj *iObj) String(name string) api.String {
	obj.Lock()
	defer obj.Unlock()

	s, ok := obj.fields[name].(*iString)
	if !ok {
		s = &iString{name: name, obj: obj}
		obj.fields[name] = s
	}
	return s
}

// Histogram
type iHistogram struct {
	name   string
	obj    *iObj
	ranges []int64
	values map[int64]int64
}

// SetRanges allows user to specify ranges for a histogram metric
func (h *iHistogram) SetRanges(ranges []int64) api.Histogram {
	if len(ranges) == 0 || len(ranges) >= 24 {
		panic(fmt.Sprintf("invalid range length: %+v", ranges))
	}
	for i := 0; i < len(ranges)-1; i++ {
		if ranges[i] >= ranges[i+1] {
			panic(fmt.Sprintf("non increasing range values: %+v", ranges))
		}
	}
	h.setHistogramRange(ranges)

	return h
}

// setHistogramRange is internal routine to update the histogram ranges
// from default or user specified
func (h *iHistogram) setHistogramRange(ranges []int64) {
	length := len(ranges)
	h.ranges = make([]int64, length+1)
	copy(h.ranges[0:length], ranges)
	h.ranges[length] = math.MaxInt64

	h.values = make(map[int64]int64, len(h.ranges))
	for i := 0; i < len(h.ranges); i++ {
		key := h.ranges[i]
		h.values[key] = 0
	}
}

// Histogram creates metric that represent a distribution
func (obj *iObj) Histogram(name string) api.Histogram {
	obj.Lock()
	defer obj.Unlock()

	h, ok := obj.fields[name].(*iHistogram)
	if !ok {
		ranges := make([]int64, defaultRanges)
		h = &iHistogram{name: name, obj: obj}
		for i := 0; i < defaultRanges; i++ {
			ranges[i] = int64(math.Exp2((float64)(2 * (i + 1))))
		}
		obj.fields[name] = h
		h.setHistogramRange(ranges)
	}
	return h
}

// AddSample adds a new sample to a distribution
func (h *iHistogram) AddSample(value int64) {
	obj := h.obj
	obj.Lock()
	defer obj.Unlock()

	idx := 0
	for i := 0; i < len(h.ranges); i++ {
		if value < h.ranges[i] {
			idx = i
			break
		}
	}
	key := h.ranges[idx]
	h.values[key]++

	obj.dirty = true
}

// Summary
type iSummary struct {
	name       string
	obj        *iObj
	totalCount int64
	totalValue float64
}

// Summary creates a metric that tracks averages
func (obj *iObj) Summary(name string) api.Summary {
	obj.Lock()
	defer obj.Unlock()

	s, ok := obj.fields[name].(*iSummary)
	if !ok {
		s = &iSummary{name: name, obj: obj}
		obj.fields[name] = s
	}
	return s
}

// AddSample adds a new sample to a distribution
func (s *iSummary) AddSample(value float64) {
	obj := s.obj
	obj.Lock()
	defer obj.Unlock()

	s.totalCount++
	s.totalValue += value

	obj.dirty = true
}

// Point contains tags and fields, used to add multiple points
type Point struct {
	Tags   map[string]string
	Fields map[string]interface{}
}

// Points add multiple points in tsdb client
func (obj *iObj) Points(points []*Point, ts time.Time) error {
	if global == nil {
		log.Errorf("tsdb is not initialized")
		return nil
	}

	if ts.IsZero() {
		ts = time.Now()
	}

	for _, v := range points {
		createNewMetricPointFromKeysFields(obj, v.Tags, v.Fields, ts)
	}
	obj.dirty = true

	return nil
}
