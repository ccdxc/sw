package tsdb

import (
	"fmt"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
)

const (
	// channelBuffer stores intermediate data structures before they are
	// transmitted over the network by sender thread
	channelBuffer = 4000
)

var gInit sync.Once

// Table interface allows adding points in the table and deletion of the table
type Table interface {
	AddPoints(name string, points []Point) error
	Delete() error
}

// TableObj is a named object on a table to which points are added and deleted
type TableObj interface {
	AddObjPoint(obj interface{}) error
	Delete() error
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

// Counter is an unsigned integer metric that is expected to continuously increase over time
type Counter uint64

// Increment is an unsigned integer metric that is reported in increments
// Using this time is dependent on how data collection is happening
type Increment uint64

// Gauge is an integer value that can go up and down on a scale
type Gauge float64

// Flag is a boolean that can be turned on or off
type Flag bool

// String is an arbitrary sized string that is measured against a field
type String string

// table represents a collection of records (aka set of Points)
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

// NewTable returns a table to the user, on which Points would be added
func NewTable(name string, config Config) (Table, error) {

	if name == "" {
		return nil, fmt.Errorf("invalid configuration params")
	}
	gInit.Do(startTransmitter)

	if _, ok := gctx.tables[name]; ok {
		return nil, fmt.Errorf("table '%s' already exists", name)
	}

	tbl := &table{Mutex: sync.Mutex{}, name: name, config: config}

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
	gctx.txChan <- points

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

// global runtime information maintained on per process basis
type globalContext struct {
	sync.Mutex

	// txChan is the channel where all message from caller are sent to the sender thread
	txChan chan interface{}

	// list of all tables maintained by the system
	tables map[string]*table
}

var gctx *globalContext

// startTransmitter function receives the objects over tx channel and sends the data over the
// network to collector(s)
func startTransmitter() {
	gctx = &globalContext{Mutex: sync.Mutex{},
		txChan: make(chan interface{}, 4000),
		tables: make(map[string]*table)}
	go func() {
		for {
			select {
			case v, ok := <-gctx.txChan:
				if !ok {
					log.Errorf("Error reading from channel. Closing watch")
					close(gctx.txChan)
					return
				}
				points, ok := v.([]Point)
				if !ok {
					log.Errorf("Invalid objects received over the channel")
					continue
				}
				if err := sendPoints(points); err != nil {
					log.Errorf("error posting points")
				}
			}
		}
	}()
}

func sendPoints(points []Point) error {
	log.Infof("received following points to be sent over network: %+v", points)

	return nil
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
	skipNames := []string{"Kind", "UUID", "ResourceVersion", "Name", "Namespace", "Tenant", "Labels", "CTime", "MTime"}
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
