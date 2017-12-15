package tsdb

import (
	"context"
	"fmt"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	// channelBuffer stores intermediate data structures before they are
	// transmitted over the network by sender thread
	channelBuffer       = 4000
	defaultDBName       = "pensandodb"
	defaultSendInterval = 2 * time.Second
)

// Options define configurable/operational parameters for transmitter interaction
type Options struct {
	ClientName     string             // ClientName for connection to Collector
	ResolverClient resolver.Interface // Resolver for getting the whereabouts of collector
	Collector      string             // name of custom collector if not using the defaults
	DBName         string             // DBName in the influx to use if not using the defaults
	SendInterval   time.Duration      // custom send interval if not using the defaults
	// TBD: reconnection period
	// TBD: should any parameters be promoted up to global level from a table?
}

// Transmitter interface allows sending points (push) to a transmitter
// (or a set of transmitter)
type Transmitter interface {
	SendPoints(dbName string, points []Point) error
	Init(opts Options) error
	Update(opts Options) error
}

// Table interface allows adding points in the table and deletion of the table
type Table interface {
	AddPoints(name string, points []Point) error
	Delete() error
}

// TableObj is a named object on a table to which points are added and deleted
type TableObj interface {
	AddObjPoint(obj interface{}) error
	Delete() error
	AddPoints(points []Point) error
	Tags() map[string]string
}

// Config specifies a table's attributes
type Config struct {
	// Precision is the write precision of the points. Permitted values are 'ns', 'us',
	// 'ms', or 's' for nano seconds, micro seconds, milli seconds or seconds. Defaults to 'ms'
	Precision string

	// Reliable flag indicates whether the table needs stats to be never dropped
	Reliable bool
}

// Point is an instance of a record within a table
type Point struct {
	// Tags are opaque strings associated with the point
	// Later used for searching points with specific tags
	Tags map[string]string

	// Fields are the values specified by the user against the series name
	// Field value can be of any type e.g. string, counter, boolean, etc.
	Fields map[string]interface{}

	// time could be filled in by the caller (esp if the points were batched and cached
	// if time is zero it is filled in by the api when a point is written
	Time time.Time
}

// Field Types: Counter, Increment, Gauge, Flag, or String defines the type of data
// associated with the field. Usually it can be any primitive type, but defining the
// specific types allows the interpretation of the fields accordingly

// Counter is an signed integer metric that is expected to continuously increase over time
//	This is NOT unsigned because influx does not support uint64
type Counter int64

// Increment is an  integer metric that is reported in increments
// Using this time is dependent on how data transmission is happening
type Increment int64

// Gauge is an integer value that can go up and down on a scale
type Gauge float64

// Flag is a boolean that can be turned on or off
type Flag bool

// String is an arbitrary sized string that is measured against a field
type String string

// table represents a transmission of records (aka set of Points)
// All records within a table have same tag-keys and field names, while their values differ
// Each record represents a time series with multiple entries sampled at different times
type table struct {
	sync.Mutex

	// name is a unique table identifier by the caller
	// The uniqueness is needed among various databases from a caller
	name string

	// desired configuration for the table
	config Config

	// object time series stats that are part of this table
	objs map[string]*tableObj

	// table statistics
	addPointCalls uint64
	totalPoints   uint64
}

// tableObj is an object that is defined in a table, an object is an
// instantiation of an kind of an object
type tableObj struct {
	// name of the table object e.g. 'tenant-foo.namespace-bar.obj-gunk'
	name string

	// tags are the cached field values for a specified object
	// used only for Obj routines
	tags map[string]string

	// reference to the table
	tbl *table
}

// Init initializes the transmitter interface
func Init(tsmt Transmitter, options Options) error {
	if gctx != nil {
		return fmt.Errorf("client already initialized")
	}

	if err := tsmt.Init(options); err != nil {
		return err
	}
	gctx = &globalContext{tsmt: tsmt,
		txChan: make(chan interface{}, channelBuffer),
		tables: make(map[string]*table)}
	gctx.Add(1)
	go startTransmitter()

	return nil
}

// Close and free any resources created in Init
func Close() {
	if gctx != nil {
		close(gctx.txChan)
		gctx.Wait()
		gctx = nil
	}
}

// NewTable returns a table to the user, on which Points would be added
func NewTable(name string, config Config) (Table, error) {

	if name == "" {
		return nil, fmt.Errorf("invalid configuration params")
	}
	if gctx == nil {
		return nil, fmt.Errorf("Uninitialized client")
	}

	if _, ok := gctx.tables[name]; ok {
		return nil, fmt.Errorf("table '%s' already exists", name)
	}

	tbl := &table{name: name, config: config}

	gctx.Lock()
	defer gctx.Unlock()

	gctx.tables[name] = tbl

	return tbl, nil
}

// AddPoints adds elements to the time series on the specified table
// Table initialization is one time operation where as adding points to it done
// as many times as desired
func (tbl *table) AddPoints(name string, points []Point) error {
	tbl.Lock()
	defer tbl.Unlock()

	if gtbl, ok := gctx.tables[tbl.name]; !ok || gtbl != tbl {
		return fmt.Errorf("table '%s' is deleted or moved", name)
	}

	tbl.addPointCalls++
	timeNow := time.Now()
	for idx := range points {
		tbl.totalPoints++
		if points[idx].Time.IsZero() {
			points[idx].Time = timeNow
		}
	}

	// TBD: do we make a copy of the objects; copying decreases performances but can be thread-safe
	gctx.Lock()
	gctx.txChan <- tbl.name
	gctx.txChan <- points
	gctx.Unlock()

	return nil
}

// Delete deletes the table in the backend, user will no longer be able to add Points to the table
// after this call
func (tbl *table) Delete() error {
	gctx.Lock()
	if tbl != nil {
		delete(gctx.tables, tbl.name)
	}
	gctx.Unlock()

	return nil
}

// NewTableObj creates an object specific table, returns ObjectTable on which ObjectPoints can be added
func NewTableObj(obj interface{}, config Config) (TableObj, error) {
	refCtx := &ref.RfCtx{GetSubObj: ref.NilSubObj}
	kvs := make(map[string]ref.FInfo)
	ref.GetKvs(obj, refCtx, kvs)

	v, ok := kvs["Kind"]
	if !ok || v.ValueStr[0] == "" {
		return nil, fmt.Errorf("error finding object kind: kvs = %+v", kvs)
	}

	tbl, ok := gctx.tables[v.ValueStr[0]]
	if !ok {
		t, err := NewTable(v.ValueStr[0], config)
		if err != nil {
			return nil, err
		}
		tbl = t.(*table)
		tbl.objs = make(map[string]*tableObj)
	}

	tbl.Lock()
	defer tbl.Unlock()

	tblObj := &tableObj{tbl: tbl, tags: make(map[string]string)}

	if err := fillKeys(kvs, tblObj.tags); err != nil {
		return nil, err
	}
	tblObj.name = ""
	for _, tagVal := range tblObj.tags {
		tblObj.name += tagVal + "."
	}
	tblObj.name = strings.TrimSuffix(tblObj.name, ".")
	tbl.objs[tblObj.name] = tblObj

	return tblObj, nil
}

// Deletes a specific object within a table
func (tblObj *tableObj) Delete() error {
	tbl := tblObj.tbl

	tbl.Lock()
	defer tbl.Unlock()

	delete(tbl.objs, tblObj.name)

	return nil
}

// AddObjPoint adds a point in time series for an object
func (tblObj *tableObj) AddObjPoint(obj interface{}) error {
	refCtx := &ref.RfCtx{GetSubObj: ref.NilSubObj}
	kvs := make(map[string]ref.FInfo)
	ref.GetKvs(obj, refCtx, kvs)

	point := Point{Tags: tblObj.tags}
	point.Fields = make(map[string]interface{})
	if err := fillFields(kvs, point.Fields); err != nil {
		return err
	}
	return tblObj.tbl.AddPoints(tblObj.name, []Point{point})
}
func (tblObj *tableObj) Tags() map[string]string {
	return tblObj.tags
}
func (tblObj *tableObj) AddPoints(points []Point) error {
	return tblObj.tbl.AddPoints(tblObj.name, points)
}

type oneFloat struct {
	Kind string
	api.ObjectMeta
}

// LogField to send a single point to tsdb
func LogField(kind string, objectMeta api.ObjectMeta, fieldName string, value interface{}) {
	to := &oneFloat{
		Kind:       kind,
		ObjectMeta: objectMeta,
	}

	tblObj, err := NewTableObj(&to, Config{})
	if err == nil {
		point := Point{Tags: tblObj.Tags(), Fields: make(map[string]interface{})}
		point.Fields[fieldName] = value
		tblObj.AddPoints([]Point{point})
	}
	tblObj.Delete()
}

// global runtime information maintained on per process basis
type globalContext struct {
	sync.Mutex
	sync.WaitGroup

	// Transmitter is an interface to communicate with a transmitter entity
	tsmt Transmitter

	// txChan is the channel where all message from caller are sent to the sender thread
	txChan chan interface{}

	// list of all tables maintained by the system
	tables map[string]*table
}

var gctx *globalContext

// startTransmitter function receives the objects over tx channel and sends the data over the
// network to transmitter(s)
func startTransmitter() {

	defer gctx.Done()
	for {
		select {
		case v, ok := <-gctx.txChan:
			if !ok {
				log.Errorf("txChan closed. Returning from transmitter goroutine")
				return
			}
			// first comes the table name, then points
			tblName, ok := v.(string)
			if !ok {
				log.Errorf("Invalid objects received over the channel")
				continue
			}

			// next read the points for the table
			v, ok = <-gctx.txChan
			if !ok {
				log.Errorf("Error reading points, closing channel")
				return
			}

			points, ok := v.([]Point)
			if !ok {
				log.Errorf("Invalid objects received over the channel")
				continue
			}
			if err := gctx.tsmt.SendPoints(tblName, points); err != nil {
				log.Errorf("error posting points")
			}
		}
	}
}

func fillKeys(kvs map[string]ref.FInfo, tags map[string]string) error {
	keyNames := []string{"Name", "Namespace", "Tenant"}
	for _, key := range keyNames {
		if v, ok := kvs[key]; ok {
			if v.ValueStr[0] != "" {
				tags[key] = v.ValueStr[0]
			}
		}
	}

	if len(tags) == 0 {
		return fmt.Errorf("keys not found")
	}

	return nil

}

func fillFields(kvs map[string]ref.FInfo, fields map[string]interface{}) error {
	// TBD: deletion of skipNames can be avoided if caller passes just the spec
	skipNames := []string{"Kind", "UUID", "ResourceVersion", "Name", "Namespace", "Tenant", "Labels", "Nanos", "Seconds"}
	for _, key := range skipNames {
		delete(kvs, key)
	}

	for key, v := range kvs {
		if v.ValueStr[0] == "" {
			continue
		}
		switch v.TypeStr {
		case "string":
			fields[key] = v.ValueStr[0]
		case "int64", "int32":
			intVal, _ := strconv.ParseInt(v.ValueStr[0], 10, 64)
			fields[key] = intVal
		case "uint64", "uint32":
			uintVal, _ := strconv.ParseUint(v.ValueStr[0], 10, 64)
			fields[key] = uintVal
		case "float32", "float64":
			floatVal, _ := strconv.ParseFloat(v.ValueStr[0], 64)
			fields[key] = floatVal
		case "bool":
			flag := false
			if v.ValueStr[0] == "true" {
				flag = true
			}
			fields[key] = flag
		default:
			return fmt.Errorf("unrecognized type %+v", v.TypeStr)
		}
	}

	return nil
}

// DummyTransmitter is a dummy transmitter which just prints to stdout
type DummyTransmitter struct {
}

// Init is a dummy routine
func (c DummyTransmitter) Init(opts Options) error {
	return nil
}

// Update is a dummy routine
func (c DummyTransmitter) Update(opts Options) error {
	return nil
}

// SendPoints send points
func (c DummyTransmitter) SendPoints(dbName string, points []Point) error {
	return nil
}

// BatchTransmitter is a transmitter which sends events to collector running on venice
//  by batching them for sendInterval
type BatchTransmitter struct {
	dbName       string
	sendInterval time.Duration
	rpcClient    *rpckit.RPCClient
	mc           metric.MetricApiClient
	metricBundle *metric.MetricBundle
	metrics      []*metric.MetricPoint
	sync.Mutex
	context context.Context
}

// NewBatchTransmitter returns a new instance of BatchTransmitter
func NewBatchTransmitter(ctx context.Context) *BatchTransmitter {
	return &BatchTransmitter{
		dbName:       defaultDBName,
		sendInterval: defaultSendInterval,
		metricBundle: &metric.MetricBundle{},
		context:      ctx,
	}
}

// Init sets up the rpc client
func (c *BatchTransmitter) Init(opts Options) error {

	if opts.SendInterval != 0 {
		c.sendInterval = opts.SendInterval
	}
	if opts.DBName != "" {
		c.dbName = opts.DBName
	}
	collector := globals.Collector
	if opts.Collector != "" {
		collector = opts.Collector
	}

	rpckitOpts := make([]rpckit.Option, 0)
	if opts.ResolverClient != nil {
		rpckitOpts = append(rpckitOpts, rpckit.WithBalancer(balancer.New(opts.ResolverClient)))
	}
	rpckitOpts = append(rpckitOpts, rpckit.WithLoggerEnabled(false))
	go c.waitForClientConn(opts.ClientName, collector, rpckitOpts)

	return nil
}
func (c *BatchTransmitter) waitForClientConn(clientName, collector string, rpckitOpts []rpckit.Option) {
	var err error
	for {
		c.rpcClient, err = rpckit.NewRPCClient(clientName, collector, rpckitOpts...)
		if err == nil {
			break
		}

		select {
		case <-time.After(c.sendInterval):
		case <-c.context.Done():
			log.Infof("returning from BatchTransmitter before establishing rpcclient conn as context completed")
			return
		}
	}
	c.mc = metric.NewMetricApiClient(c.rpcClient.ClientConn)
	c.metricBundle.DbName = c.dbName
	c.metricBundle.Reporter = clientName
	go c.sendPoints()
}

// Update is a dummy routine
func (c *BatchTransmitter) Update(opts Options) error {
	return nil
}
func xformFields(p *Point) map[string]*metric.Field {
	res := make(map[string]*metric.Field)
	for k, v := range p.Fields {
		switch v.(type) {
		case int64:
			val := v.(int64)
			res[k] = &metric.Field{
				F: &metric.Field_Counter{
					Counter: uint64(val),
				},
			}
		case Counter:
			val := v.(Counter)
			res[k] = &metric.Field{
				F: &metric.Field_Counter{
					Counter: uint64(val),
				},
			}
		case float64:
			val := v.(float64)
			res[k] = &metric.Field{
				F: &metric.Field_Gauge{
					Gauge: float64(val),
				},
			}

		case Gauge:
			val := v.(Gauge)
			res[k] = &metric.Field{
				F: &metric.Field_Gauge{
					Gauge: float64(val),
				},
			}
		case string:
			val := v.(string)
			res[k] = &metric.Field{
				F: &metric.Field_String_{
					String_: val,
				},
			}

		case String:
			val := v.(String)
			res[k] = &metric.Field{
				F: &metric.Field_String_{
					String_: string(val),
				},
			}
		case bool:
			val := v.(bool)
			res[k] = &metric.Field{
				F: &metric.Field_Bool{
					Bool: bool(val),
				},
			}
		case Flag:
			val := v.(Flag)
			res[k] = &metric.Field{
				F: &metric.Field_Bool{
					Bool: bool(val),
				},
			}
		default:
			log.Infof("tsdb.xformFields: Ignoring unknown type %s\n", v)

		}
	}
	return res
}

func (c *BatchTransmitter) sendPoints() {
	for {
		select {
		case <-time.After(c.sendInterval):
			c.Lock()
			c.metricBundle.Metrics = c.metrics
			c.metrics = nil
			c.Unlock()
			if len(c.metricBundle.Metrics) == 0 {
				continue
			}
			_, err := c.mc.WriteMetrics(c.context, c.metricBundle)
			if err != nil {
				log.Errorf("sendPoints : WriteMetrics failed with err: %s", err)
			}
		case <-c.context.Done():
			log.Infof("returning from BatchTransmitter as context completed")
			return
		}
	}
}

// SendPoints send points
func (c *BatchTransmitter) SendPoints(dbName string, points []Point) error {
	var err error
	tt, _ := types.TimestampProto(time.Now())

	for _, p := range points {
		f := xformFields(&p)
		if len(f) == 0 {
			continue
		}

		var t *types.Timestamp
		if p.Time.IsZero() {
			t = tt
		} else {
			t, err = types.TimestampProto(p.Time)
			if err != nil {
				t = tt
			}
		}

		mp := &metric.MetricPoint{
			Name:   dbName,
			Tags:   p.Tags,
			Fields: f,
			When: &api.Timestamp{
				Timestamp: *t,
			},
		}
		c.Lock()
		c.metrics = append(c.metrics, mp)
		c.Unlock()
	}
	return err
}
