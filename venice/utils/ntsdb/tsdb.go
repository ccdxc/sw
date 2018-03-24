package ntsdb

import (
	"context"
	"fmt"
	"math"
	"net/http"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	defaultRanges                  = 10
	defaultSendInterval            = 10 * time.Second
	defaultConnectionRetryInterval = 100 * time.Millisecond
)

type globalInfo struct {
	opts          Opts                   // user options
	sync.Mutex                           // global lock
	rpcClient     *rpckit.RPCClient      // rpc connection to collector
	mc            metric.MetricApiClient // metric client object (to write points)
	context       context.Context        // global context (used for cancellation)
	tables        map[string]*iTable     // various tables created by user
	deletedTables []*iTable              // deleted list of tables
	httpServer    *http.Server           // local http server
	cancelFunc    context.CancelFunc     // cancel function to initiate internal cleanup
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
	LocalPort               int                // when non zero we would start a REST server to fetch local metrics
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
		opts.SendInterval = defaultSendInterval
	}
	if opts.ConnectionRetryInterval == 0 {
		opts.ConnectionRetryInterval = defaultConnectionRetryInterval
	}

	global.tables = make(map[string]*iTable)
	global.context, global.cancelFunc = context.WithCancel(ctx)

	go periodicTransmit()
	go startLocalRESTServer()
}

// Cleanup could be called for ephemeral processes using tsdb
func Cleanup() {
	if global == nil {
		return
	}

	// flush any metrics that are not yet pushed out
	sendAllTables()

	// cancel the context to stop any running go threads
	global.cancelFunc()
	stopLocalRESTServer()

	global = nil
}

type timeField struct {
	ts     time.Time
	field  interface{}
	subIdx int
}

// Table represents a measurement
type Table interface {
	Delete()
	Counter(field string) api.Counter
	Gauge(field string) api.Gauge
	String(field string) api.String
	Bool(field string) api.Bool
	Histogram(field string) api.Histogram
	Summary(field string) api.Summary
	Point(keys map[string]string, fields map[string]interface{}, ts time.Time)
}

// TableOpts specifies options for a specific table
type TableOpts struct {
	Local bool
}

type iTable struct {
	sync.Mutex
	name       string
	opts       TableOpts
	keys       map[string]string
	fields     map[string]interface{}
	timeFields []*timeField
	dirty      bool
}

// NewTable creates a freeform table that can be used to record arbitrary fields
func NewTable(name string, opts *TableOpts) (Table, error) {
	t := &iTable{}
	t.name = name
	t.keys = make(map[string]string)
	t.keys["Name"] = name
	t.fields = make(map[string]interface{})
	t.opts = *opts

	global.Lock()
	defer global.Unlock()
	if _, ok := global.tables[name]; ok {
		return nil, fmt.Errorf("table already exist")
	}
	global.tables[name] = t

	return t, nil
}

// NewOTable creates a table for the specified object and its metrics
func NewOTable(obj interface{}, metrics interface{}, opts *TableOpts) (Table, error) {
	keys := make(map[string]string)
	tableName, err := getKeys(obj, keys)
	if err != nil {
		return nil, err
	}

	table, err := NewTable(tableName, opts)
	if err != nil {
		return nil, err
	}
	t := table.(*iTable)

	t.keys = keys
	if err := fillFields(t, metrics); err != nil {
		return nil, err
	}

	return t, nil
}

// Delete cleans up resources associated with the table
func (table *iTable) Delete() {
	global.Lock()
	defer global.Unlock()
	global.deletedTables = append(global.deletedTables, table)
	delete(global.tables, table.name)
}

// Counter
type iCounter struct {
	name    string
	table   *iTable
	value   int64
	nextIdx int
}

// Add increments the counter by the specified value
func (c *iCounter) Add(inc int64) {
	table := c.table
	table.Lock()
	defer table.Unlock()

	c.value += inc
	tf := &timeField{ts: time.Now(), field: c}
	table.timeFields = append(table.timeFields, tf)
	c.nextIdx++

	table.dirty = true
}

// Add increments the counter by one
func (c *iCounter) Inc() {
	table := c.table
	table.Lock()
	defer table.Unlock()

	c.value++
	tf := &timeField{ts: time.Now(), field: c}
	table.timeFields = append(table.timeFields, tf)
	c.nextIdx++

	table.dirty = true
}

// Counter function creates and returns a new counter metric
func (table *iTable) Counter(name string) api.Counter {
	table.Lock()
	defer table.Unlock()

	c, ok := table.fields[name].(*iCounter)
	if !ok {
		c = &iCounter{name: name, table: table}
		table.fields[name] = c
	}
	return c
}

// Gauge
type iGauge struct {
	name    string
	table   *iTable
	values  []float64
	nextIdx int
}

// Set registers a value of a guage
func (g *iGauge) Set(val float64, ts time.Time) {
	table := g.table

	table.Lock()
	defer table.Unlock()

	g.values = append(g.values, val)
	if ts.IsZero() {
		ts = time.Now()
	}
	tf := &timeField{ts: ts, field: g, subIdx: g.nextIdx}
	table.timeFields = append(table.timeFields, tf)
	g.nextIdx++

	table.dirty = true
}

// Gauge creates a gauge metric
func (table *iTable) Gauge(name string) api.Gauge {
	table.Lock()
	defer table.Unlock()

	g, ok := table.fields[name].(*iGauge)
	if !ok {
		g = &iGauge{name: name, table: table}
		table.fields[name] = g
	}
	return g
}

// Bool
type iBool struct {
	name    string
	table   *iTable
	values  []bool
	nextIdx int
}

// Set registers a value of a bool
func (b *iBool) Set(val bool, ts time.Time) {
	table := b.table
	table.Lock()
	defer table.Unlock()

	b.values = append(b.values, val)
	if ts.IsZero() {
		ts = time.Now()
	}
	tf := &timeField{ts: ts, field: b, subIdx: b.nextIdx}
	table.timeFields = append(table.timeFields, tf)
	b.nextIdx++

	table.dirty = true
}

// Bool creates a boolean metric
func (table *iTable) Bool(name string) api.Bool {
	table.Lock()
	defer table.Unlock()

	b, ok := table.fields[name].(*iBool)
	if !ok {
		b = &iBool{name: name, table: table}
		table.fields[name] = b
	}
	return b
}

// String
type iString struct {
	name    string
	table   *iTable
	nextIdx int
	values  []string
}

// Set registers a value of a bool
func (s *iString) Set(val string, ts time.Time) {
	table := s.table
	table.Lock()
	defer table.Unlock()

	s.values = append(s.values, val)
	if ts.IsZero() {
		ts = time.Now()
	}
	tf := &timeField{ts: ts, field: s, subIdx: s.nextIdx}
	table.timeFields = append(table.timeFields, tf)
	s.nextIdx++

	table.dirty = true
}

// String creates a string metric
func (table *iTable) String(name string) api.String {
	table.Lock()
	defer table.Unlock()

	s, ok := table.fields[name].(*iString)
	if !ok {
		s = &iString{name: name, table: table}
		table.fields[name] = s
	}
	return s
}

// Histogram
type iHistogram struct {
	name    string
	table   *iTable
	ranges  []int64
	values  map[int64]int64
	nextIdx int
	dirty   map[int64]bool
}

// SetRanges allows setting ranges for a histogram metric
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

func (h *iHistogram) setHistogramRange(ranges []int64) {
	length := len(ranges)
	h.ranges = make([]int64, length+1)
	copy(h.ranges[0:length], ranges)
	h.ranges[length] = math.MaxInt64

	h.values = make(map[int64]int64, len(h.ranges))
	h.dirty = make(map[int64]bool, len(h.ranges))
	for i := 0; i < len(h.ranges); i++ {
		key := h.ranges[i]
		h.values[key] = 0
		h.dirty[key] = false
	}
}

// Histogram creates metric that represent a distribution
func (table *iTable) Histogram(name string) api.Histogram {
	table.Lock()
	defer table.Unlock()

	h, ok := table.fields[name].(*iHistogram)
	if !ok {
		ranges := make([]int64, defaultRanges)
		h = &iHistogram{name: name, table: table}
		for i := 0; i < defaultRanges; i++ {
			ranges[i] = (int64)(math.Exp2((float64)(2 * (i + 1))))
		}
		table.fields[name] = h
		h.setHistogramRange(ranges)
	}
	return h
}

// AddSample adds a new sample to a distribution
func (h *iHistogram) AddSample(value int64) {
	table := h.table
	table.Lock()
	defer table.Unlock()

	idx := 0
	for i := 0; i < len(h.ranges); i++ {
		if value < h.ranges[i] {
			idx = i
			break
		}
	}
	key := h.ranges[idx]
	h.values[key]++

	tf := &timeField{ts: time.Now(), field: h}
	table.timeFields = append(table.timeFields, tf)
	h.nextIdx++

	h.dirty[key] = true
	table.dirty = true
}

// Summary
type iSummary struct {
	name       string
	table      *iTable
	totalCount int64
	totalValue float64
	nextIdx    int
}

// Summary creates a metric that tracks averages
func (table *iTable) Summary(name string) api.Summary {
	table.Lock()
	defer table.Unlock()

	s, ok := table.fields[name].(*iSummary)
	if !ok {
		s = &iSummary{name: name, table: table}
		table.fields[name] = s
	}
	return s
}

// AddSample adds a new sample to a distribution
func (s *iSummary) AddSample(value float64) {
	table := s.table
	table.Lock()
	defer table.Unlock()

	s.totalCount++
	s.totalValue += value

	tf := &timeField{ts: time.Now(), field: s, subIdx: s.nextIdx}
	table.timeFields = append(table.timeFields, tf)
	s.nextIdx++

	table.dirty = true
}

// Point
type iPoint struct {
	keys   map[string]string
	fields map[string]interface{}
	table  *iTable
}

// Point creates a point with keys/fields in the time series
func (table *iTable) Point(keys map[string]string, fields map[string]interface{}, ts time.Time) {
	table.Lock()
	defer table.Unlock()

	if ts.IsZero() {
		ts = time.Now()
	}

	p := &iPoint{keys: keys, fields: fields, table: table}
	tf := &timeField{ts: ts, field: p}
	table.timeFields = append(table.timeFields, tf)

	table.dirty = true
}
