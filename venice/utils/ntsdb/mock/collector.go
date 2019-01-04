package mock

import (
	"context"
	"fmt"
	"reflect"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/citadel/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	delim = "_"
)

// Collector is a fake collector saving all received metric bundles
type Collector struct {
	prevWrite time.Time
	lastWrite time.Time
	appends   int
	mb        *metric.MetricBundle
}

// WriteMetrics method allows implementing collector interface
func (f *Collector) WriteMetrics(ctx context.Context, mb *metric.MetricBundle) (*api.Empty, error) {
	f.prevWrite = f.lastWrite
	f.lastWrite = time.Now()
	if f.mb != nil {
		for _, mp := range mb.Metrics {
			f.mb.Metrics = append(f.mb.Metrics, mp)
		}
	} else {
		f.mb = mb
	}
	f.appends++

	return &api.Empty{}, nil
}

// WriteLines method allows for writing line protocol towards collector
func (f *Collector) WriteLines(ctx context.Context, in *metric.LineBundle) (*api.Empty, error) {
	return &api.Empty{}, nil
}

// ValidateSendInterval gets the time when last write function was called
func (f *Collector) ValidateSendInterval(expDuration time.Duration) bool {
	actualDuration := f.lastWrite.Sub(f.prevWrite)
	deviation := float64(actualDuration) / float64(expDuration)

	if deviation > 8 || deviation < 1 {
		log.Errorf("actualDuration %d expDuration %d deviation %f", actualDuration, expDuration, deviation)
		return false
	}

	f.ClearMetrics()
	return true
}

// ClearMetrics clears any cached metrics in collector
func (f *Collector) ClearMetrics() {
	f.mb = nil
	f.appends = 0
}

// Validate ensures that fake collector has received the specified tags/fields over measurement time
func (f *Collector) Validate(measurementName string, ts time.Time, tags []map[string]string, fieldsRanges []map[string]interface{}) bool {
	if f.mb == nil {
		log.Errorf("nil metric bundle ")
		return false
	}

	if f.mb.DbName != "objMetrics" {
		log.Errorf("unexpected dbName, got '%s' expected '%s'", f.mb.DbName, "objMetrics")
		return false
	}

	if len(f.mb.Metrics) != len(fieldsRanges) || len(f.mb.Metrics) != len(tags) {
		log.Errorf("mismatching metrics count, received %d expected %d tags, %d fields \n", len(f.mb.Metrics), len(fieldsRanges), len(tags))
		log.Errorf("appends %d tags = %+v\n", f.appends, tags)
		for ii := range f.mb.Metrics {
			log.Errorf("Got Metric[%d] %+v\n", ii, f.mb.Metrics[ii])
		}
		return false
	}

	for _, mp := range f.mb.Metrics {
		if mp.When == nil {
			log.Errorf("received time in Metric is nil")
			return false
		}
		if mp.When.Seconds == 0 && mp.When.Nanos == 0 {
			log.Errorf("received time in Metric is zero")
			return false
		}
		if mp.Name == "" {
			log.Errorf("received name in Metric is nil")
			return false
		}
		if len(mp.Fields) == 0 {
			log.Errorf("No fields received in Metric")
			return false

		}
		if !ts.IsZero() && (mp.When.Seconds != int64(ts.Unix()) || mp.When.Nanos != int32(ts.Nanosecond())) {
			log.Errorf("mismatching ts: secs %v (exp %v) nsecs %v (exp %v)", mp.When.Seconds, ts.Unix(), mp.When.Nanos, ts.Nanosecond())
			return false
		}
		mp.When = nil
		match := false
		emps := make([]*metric.MetricPoint, 0)
		for fIdx, fields := range fieldsRanges {
			emp := &metric.MetricPoint{
				Name:   measurementName,
				Tags:   tags[fIdx],
				Fields: getMf(fields),
			}

			if matchAtLeast(mp, emp) {
				match = true

				// null out the tags so we never match it again
				tags[fIdx] = map[string]string{}
				break
			}
			emps = append(emps, emp)
		}
		if !match {
			log.Error("Mismatching metrics")
			printMp("Got:", mp)
			for idx, m := range emps {
				printMp(fmt.Sprintf("Expected idx %d", idx), m)
			}
			return false
		}
	}
	f.ClearMetrics()
	return true
}

// matches at least all the fields in emp (expected metric points) to be present in mp (metric points)
// returns true if it matches
func matchAtLeast(mp, emp *metric.MetricPoint) bool {
	if emp.Name != mp.Name {
		return false
	}
	if !reflect.DeepEqual(emp.Tags, mp.Tags) {
		return false
	}
	for ek, ef := range emp.Fields {
		f, ok := mp.Fields[ek]
		if !ok {
			return false
		}
		if !reflect.DeepEqual(f, ef) {
			return false
		}
	}
	return true
}

// ValidateCount ensures that fake collector has received the specified count of tags/fields over measurement time
func (f *Collector) ValidateCount(measurementName string, tags map[string]string, numMetrics, intsPerRec, floatsPerRec, boolsPerRec, stringsPerRec int) bool {
	if f.mb == nil {
		log.Errorf("nil metric bundle ")
		return false
	}

	if f.mb.DbName != "objMetrics" {
		log.Errorf("unexpected dbName, got '%s' expected '%s'", f.mb.DbName, "objMetrics")
		return false
	}

	if numMetrics > len(f.mb.Metrics) || numMetrics+f.appends < len(f.mb.Metrics) {
		log.Errorf("#metrics mismatch exp %d got %d appends %d", numMetrics, len(f.mb.Metrics), f.appends)
		return false
	}
	exceptions := 0
	for _, mp := range f.mb.Metrics {
		if mp.When == nil {
			log.Errorf("received time in Metric is nil")
			return false
		}
		if mp.When.Seconds == 0 && mp.When.Nanos == 0 {
			log.Errorf("received time in Metric is zero")
			return false
		}
		if mp.Name == "" {
			log.Errorf("received name in Metric is nil")
			return false
		}
		if len(mp.Fields) == 0 {
			log.Errorf("No fields received in Metric")
			return false

		}
		mp.When = nil

		int64s := 0
		float64s := 0
		strings := 0
		bools := 0
		for _, value := range mp.Fields {
			v := value.F
			switch v.(type) {
			case *metric.Field_Int64:
				int64s++
			case *metric.Field_Float64:
				float64s++
			case *metric.Field_Bool:
				bools++
			case *metric.Field_String_:
				strings++
			default:
				panic(fmt.Sprintf("unknown type: %+v", v))
			}
		}
		if int64s != intsPerRec || float64s != floatsPerRec || strings != stringsPerRec || bools != boolsPerRec {
			log.Errorf("invalid recs per metric: ints (got %d exp %d), bools (got %d exp %d), strings (got %d exp %d), floats (got %d exp %d)",
				int64s, intsPerRec, bools, boolsPerRec, strings, stringsPerRec, float64s, floatsPerRec)
			log.Errorf("rec %+v", mp.Fields)
			exceptions++
		}
	}
	if exceptions > f.appends {
		log.Errorf("numExceptions: %d appends %d", exceptions, f.appends)
		return false
	}
	f.ClearMetrics()
	return true
}
func printMp(title string, mp *metric.MetricPoint) {
	log.Errorf("%s: name %+v tags %+v", title, mp.Name, mp.Tags)
	for k, mf := range mp.Fields {
		log.Errorf("key = %s, value %+v", k, mf.F)
	}
}

func getMf(fields map[string]interface{}) map[string]*metric.Field {
	mf := make(map[string]*metric.Field)
	for key, val := range fields {
		switch val.(type) {
		case float64:
			value := val.(float64)
			mf[key] = &metric.Field{
				F: &metric.Field_Float64{
					Float64: value,
				}}
		case int64:
			value := val.(int64)
			mf[key] = &metric.Field{
				F: &metric.Field_Int64{
					Int64: value,
				}}
		case string:
			value := val.(string)
			mf[key] = &metric.Field{
				F: &metric.Field_String_{
					String_: value,
				}}
		case bool:
			value := val.(bool)
			mf[key] = &metric.Field{
				F: &metric.Field_Bool{
					Bool: value,
				}}
		}
	}
	return mf
}
