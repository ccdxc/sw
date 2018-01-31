package gatherer

import (
	"fmt"
	"strconv"
	"sync"
	"time"

	"github.com/influxdata/influxdb/client/v2"

	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	bgSyncInterval = time.Second * 60
)

// shared across all query spaces
type sharedInfo struct {
	sync.RWMutex
	runtime.BG
	backends  []client.Client          // backend clients
	cToURL    map[client.Client]string // for debug
	tsdbInfo  map[string]*columnMeta   // meta data for each db<>meas
	databases map[string]bool          // databases known to gatherer
	bgSync    bool
}

type parserFunc func(string, string, map[string]string, map[string]interface{}) error

type columnMeta struct {
	columnParsers map[string]parserFunc
}

type mMeta struct {
	tags   []string
	fields map[string]string
}

// handle to gatherer shared info
var gtrMeta *sharedInfo

func init() {
	gtrMeta = &sharedInfo{}
	gtrMeta.tsdbInfo = make(map[string]*columnMeta)
	gtrMeta.databases = make(map[string]bool)
}

// SetBackends updates information about the influx backends to be queried
func SetBackends(bes []string) error {
	gtrMeta.Lock()
	defer gtrMeta.Unlock()
	gtrMeta.cToURL = make(map[client.Client]string)
	gtrMeta.backends = []client.Client{}
	for _, url := range bes {
		c, err := client.NewHTTPClient(client.HTTPConfig{
			Addr: url,
		})

		if err == nil {
			gtrMeta.backends = append(gtrMeta.backends, c)
			gtrMeta.cToURL[c] = url
		} else {
			return err
		}
	}

	return nil
}

// AddDatabase adds a database to meta
func AddDatabase(db string) {
	gtrMeta.Lock()
	defer gtrMeta.Unlock()

	gtrMeta.databases[db] = true
}

// DelDatabase deletes a db from meta
func DelDatabase(db string) {
	gtrMeta.Lock()
	defer gtrMeta.Unlock()

	delete(gtrMeta.databases, db)
}

// StartBGSync starts the background sync of tsdb meta data
func StartBGSync() {
	gtrMeta.Lock()
	defer gtrMeta.Unlock()

	if gtrMeta.bgSync {
		return
	}

	gtrMeta.Start()
	go bgSyncMeta()

	gtrMeta.bgSync = true
}

// StopBGSync stops the background sync of tsdb meta data
func StopBGSync() {
	gtrMeta.Lock()
	defer gtrMeta.Unlock()
	gtrMeta.StopAsync()
	gtrMeta.bgSync = false
}

func getDBList() []string {
	gtrMeta.RLock()
	defer gtrMeta.RUnlock()
	dbList := make([]string, 0, len(gtrMeta.databases))
	for db := range gtrMeta.databases {
		dbList = append(dbList, db)
	}

	return dbList
}

func bgSyncMeta() {
	defer gtrMeta.Done()

	for {
		select {
		case <-gtrMeta.StopCh():
			return
		case <-time.After(bgSyncInterval):
			SyncMeta()
		}
	}

}

// SyncMeta syncs the meta data used for row parsing
func SyncMeta() error {
	dbList := getDBList()
	qs := NewQuerySpace(nil)

	// get a union of tags and fields for each db:meas

	for _, db := range dbList {
		mInfo, err := qs.gatherMeta(db)
		if err != nil {
			return err
		}

		for meas, mM := range mInfo {
			CreateMeasurement(db, meas, mM.tags, mM.fields)
		}
	}

	return nil
}

func formMeasKey(db, meas string) string {
	return fmt.Sprintf("%s<>%s", db, meas)
}

// getColumnParsers returns a slice of parsers for the columns
func getColumnParsers(db, meas string, columns []string) ([]parserFunc, error) {
	mKey := formMeasKey(db, meas)

	gtrMeta.Lock()
	defer gtrMeta.Unlock()
	mm := gtrMeta.tsdbInfo[mKey]
	if mm == nil {
		return nil, fmt.Errorf("%s not found", mKey)
	}

	pf := make([]parserFunc, len(columns))

	for ix, name := range columns {
		pf[ix] = mm.columnParsers[name]
		if pf[ix] == nil {
			return nil, fmt.Errorf("%s - column %s not found", mKey, name)
		}
	}

	return pf, nil
}

// CreateMeasurement creates an entry for the measurement with the
// specified keys and fields
func CreateMeasurement(db, meas string, tagKeys []string, fields map[string]string) {
	mKey := formMeasKey(db, meas)

	gtrMeta.Lock()
	defer gtrMeta.Unlock()

	// if this is a new measurement, create it
	mm := gtrMeta.tsdbInfo[mKey]
	if mm == nil {
		mm = &columnMeta{
			columnParsers: make(map[string]parserFunc),
		}
		mm.columnParsers["time"] = parseTime
		gtrMeta.tsdbInfo[mKey] = mm
	}

	for _, t := range tagKeys {
		mm.columnParsers[t] = parseTag
	}

	for k, v := range fields {
		mm.columnParsers[k] = getParser(v)
	}
}

func parseTag(k, v string, tags map[string]string, fields map[string]interface{}) error {
	tags[k] = v
	return nil
}

func getParser(t string) parserFunc {
	switch t {
	case "integer":
		return parseInteger
	case "float":
		return parseFloat
	case "boolean":
		return parseBool
	default:
		return parseString
	}
}

func parseInteger(k, v string, tags map[string]string, fields map[string]interface{}) error {
	i, err := strconv.ParseInt(v, 10, 64)
	if err != nil {
		return err
	}

	fields[k] = i
	return nil
}

func parseFloat(k, v string, tags map[string]string, fields map[string]interface{}) error {
	f, err := strconv.ParseFloat(v, 64)
	if err != nil {
		return err
	}

	fields[k] = f
	return nil
}

func parseBool(k, v string, tags map[string]string, fields map[string]interface{}) error {
	b, err := strconv.ParseBool(v)
	if err != nil {
		return err
	}

	fields[k] = b
	return nil
}

func parseTime(k, v string, tags map[string]string, fields map[string]interface{}) error {
	time, err := time.Parse(time.RFC3339, v)
	if err != nil {
		return err
	}

	fields[k] = time
	return nil
}

func parseString(k, v string, tags map[string]string, fields map[string]interface{}) error {
	fields[k] = v
	return nil
}
