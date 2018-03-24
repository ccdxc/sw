package ntsdb

import (
	"encoding/json"
	"fmt"
	"net/http"
	"strings"
	"time"

	"github.com/pensando/sw/venice/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/utils/log"
)

func startLocalRESTServer() {
	if global.opts.LocalPort == 0 {
		return
	}
	http.HandleFunc("/", localMetricsHandler)

	global.httpServer = &http.Server{Addr: fmt.Sprintf("localhost:%d", global.opts.LocalPort)}
	err := global.httpServer.ListenAndServe()
	if err != nil {
		panic(fmt.Sprintf("unable to start local server at url '%s'", global.httpServer.Addr))
	}
}

func stopLocalRESTServer() {
	if global.opts.LocalPort == 0 {
		return
	}
	global.httpServer.Shutdown(nil)
}

// LocalMetric is the export format for local curl queries
type LocalMetric struct {
	Name       string            // Name of the metric
	Time       string            // Last Timestamp the metrics were updated/fetched (last snapshot)
	Attributes map[string]string // Values of various attributes measured for a given metric (last snapshot)
}

// Local metrics serve modules a way to fetch internal metrics gathered using the client lib
// Supported local URL format (simple two hierarchy system:
//     / 				- all tables
//     /<table-name>			- specific table, all attributes
//     /<table-name>/attribute-name	- specific table, specific attribute
func localMetricsHandler(w http.ResponseWriter, r *http.Request) {
	pathStrings := strings.Split(r.URL.Path, "/")

	tableName := ""
	attributeName := ""
	if len(pathStrings) >= 2 {
		tableName = pathStrings[1]
	}
	if len(pathStrings) >= 3 {
		attributeName = pathStrings[2]
	}

	tables := []*iTable{}
	global.Lock()
	for _, table := range global.tables {
		if (tableName == "" || tableName == table.name) && table.dirty {
			tables = append(tables, table)
		}
	}
	for _, table := range global.deletedTables {
		if tableName == "" || tableName == table.name {
			tables = append(tables, table)
		}
	}
	global.Unlock()

	if len(tables) == 0 {
		var err error
		if tableName == "" {
			err = fmt.Errorf("table %s not found", tableName)
		} else {
			err = fmt.Errorf("no tables found")
		}
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	lms := []LocalMetric{}
	for _, table := range tables {
		table.Lock()
		mb := &metric.MetricBundle{}
		table.getMetricBundles(mb)
		table.Unlock()
		fetchLms(table.name, attributeName, mb.Metrics, &lms)
	}

	lmsData, err := json.Marshal(&lms)
	if err != nil {
		log.Errorf("unable to marshal - %s", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	w.Header().Set("Content-Type", "application/json")
	w.Write(lmsData)
}

func fetchLms(tableName, attributeName string, mps []*metric.MetricPoint, lms *[]LocalMetric) {
	for _, mp := range mps {
		lm := LocalMetric{
			Name:       tableName,
			Time:       time.Unix(int64(mp.When.Seconds), int64(mp.When.Nanos)).String(),
			Attributes: map[string]string{}}
		for key, mf := range mp.Fields {
			if attributeName == "" || attributeName == key {
				lm.Attributes[key] = getString(mf)
			}
		}
		if len(lm.Attributes) > 0 {
			*lms = append(*lms, lm)
		}
	}
}

func getString(mf *metric.Field) string {
	v := mf.F
	switch v.(type) {
	case *metric.Field_Float64:
		c := v.(*metric.Field_Float64)
		return fmt.Sprintf("%v", c.Float64)
	case *metric.Field_String_:
		c := v.(*metric.Field_String_)
		return fmt.Sprintf("%v", c.String_)
	case *metric.Field_Int64:
		c := v.(*metric.Field_Int64)
		return fmt.Sprintf("%v", c.Int64)
	case *metric.Field_Bool:
		c := v.(*metric.Field_Bool)
		return fmt.Sprintf("%v", c.Bool)
	}
	return "unkonwn type"
}
