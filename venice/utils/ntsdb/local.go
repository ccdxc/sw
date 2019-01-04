package ntsdb

import (
	"encoding/json"
	"fmt"
	"net/http"
	"strings"
	"time"

	"github.com/pensando/sw/venice/citadel/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// Local Metrics
// local metrics are metrics that a process can collect for itself and not
// export it to the citadel; these metrics typically represent the internal
// statistics that could be either verbose and/or not useful for the end user
// another use case for local metrics is to know the state o a process if it
// was unable to push the elements to the citadel

// startLocalRESTServer starts a rest server to expose local metrics directly from the process
func startLocalRESTServer(global *globalInfo) {
	if global.opts.LocalPort == 0 {
		global.wg.Done()
		return
	}
	http.HandleFunc("/", localMetricsHandler)

	global.httpServer = &http.Server{Addr: fmt.Sprintf("%s:%d", globals.Localhost, global.opts.LocalPort)}
	global.wg.Done()

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
//     / 				- all objs
//     /<table-name>			- specific obj, all attributes
//     /<table-name>/attribute-name	- specific obj, specific attribute
func localMetricsHandler(w http.ResponseWriter, r *http.Request) {
	skipObjFn := func(tableName string, obj *iObj) bool {
		// when displaying all tables, skip non-dirty tables, and skip non local tables
		if tableName == "" && (!obj.dirty || !obj.opts.Local) {
			return true
		}
		// if tablename is specified then skip on mismatch
		if tableName != "" && tableName != obj.tableName {
			return true
		}
		return false
	}

	pathStrings := strings.Split(r.URL.Path, "/")

	tableName := ""
	attributeName := ""
	if len(pathStrings) >= 2 {
		tableName = pathStrings[1]
	}
	if len(pathStrings) >= 3 {
		attributeName = pathStrings[2]
	}

	objs := []*iObj{}
	global.Lock()
	for _, obj := range global.objs {
		if skipObjFn(tableName, obj) {
			continue
		}
		objs = append(objs, obj)
	}
	for _, obj := range global.deletedObjs {
		if skipObjFn(tableName, obj) {
			continue
		}
		objs = append(objs, obj)
	}
	global.Unlock()

	if len(objs) == 0 {
		var err error
		if tableName == "" {
			err = fmt.Errorf("obj %s not found", tableName)
		} else {
			err = fmt.Errorf("no objs found")
		}
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	lms := []LocalMetric{}
	for _, obj := range objs {
		obj.Lock()
		mb := &metric.MetricBundle{}
		obj.getMetricBundles(mb)
		obj.Unlock()
		fetchLms(obj.tableName, attributeName, mb.Metrics, &lms)
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

// fetchLms converts metric points to local metric format
// local metric format is kept simpler for human readability
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

// getString would fetch a string from any metric field, used to display
// local metrics in a user friendly way
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
