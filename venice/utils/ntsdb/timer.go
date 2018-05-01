package ntsdb

import (
	"fmt"
	"sync/atomic"
	"time"

	protobuf "github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

func establishConn() bool {
	for {
		var err error
		rpckitOpts := make([]rpckit.Option, 0)
		if global.opts.ResolverClient != nil {
			rpckitOpts = append(rpckitOpts, rpckit.WithBalancer(balancer.New(global.opts.ResolverClient)))
		}
		rpckitOpts = append(rpckitOpts, rpckit.WithLoggerEnabled(false))

		global.rpcClient, err = rpckit.NewRPCClient(global.opts.ClientName, global.opts.Collector, rpckitOpts...)
		if err == nil {
			break
		}
		log.Errorf("error establishing the connection: %s", err)

		select {
		case <-time.After(global.opts.ConnectionRetryInterval):
		case <-global.context.Done():
			log.Infof("aborting tsdb transmit thread: parent context ended")
			return false
		}
	}

	global.mc = metric.NewMetricApiClient(global.rpcClient.ClientConn)
	return true
}

func periodicTransmit() {
	if !establishConn() {
		return
	}

	global.wg.Add(1)
	defer global.wg.Done()
	for {
		select {
		case <-time.After(global.opts.SendInterval):
			sendAllTables()
		case <-global.context.Done():
			log.Infof("aborting tsdb transmit thread: parent context ended")
			return
		}
	}
}

func sendAllTables() {
	tables := []*iTable{}

	// fetch dirty tables
	global.Lock()
	dbName := global.opts.DBName
	for _, table := range global.tables {
		if !table.opts.Local && atomic.LoadInt32(&table.dirty) != 0 {
			tables = append(tables, table)
		}
	}
	for _, table := range global.deletedTables {
		if !table.opts.Local {
			tables = append(tables, table)
		}
	}
	global.deletedTables = []*iTable{}
	global.Unlock()

	// create metric bundle
	mb := &metric.MetricBundle{DbName: dbName, Reporter: global.opts.ClientName, Metrics: nil}
	for _, table := range tables {
		table.Lock()
		table.getMetricBundles(mb)
		table.Unlock()
	}

	// push metrics to collector
	if len(mb.Metrics) > 0 {
		_, err := global.mc.WriteMetrics(global.context, mb)
		if err != nil {
			log.Errorf("sendPoints : WriteMetrics failed with err: %s", err)
		}
	}
}

func (table *iTable) getMetricBundles(mb *metric.MetricBundle) {
	startIdx := 0

	for len(table.timeFields) > 0 {
		fields := make(map[string]*metric.Field)
		beginTime := table.timeFields[startIdx].ts
		mp := newMetricPoint(table, beginTime)

		visited := make(map[string]bool)
		idxInc := 0

	collectMetric:
		for _, tf := range table.timeFields[startIdx:] {
			v := tf.field
			switch v.(type) {
			case *iCounter:
				mfCounter(tf, fields)
			case *iGauge:
				if tf.ts.Sub(beginTime) > table.opts.Precision {
					break collectMetric
				}
				if skip := mfGauge(tf, visited, fields); skip {
					break collectMetric
				}
			case *iBool:
				if tf.ts.Sub(beginTime) > table.opts.Precision {
					break collectMetric
				}
				if skip := mfBool(tf, visited, fields); skip {
					break collectMetric
				}
			case *iString:
				if tf.ts.Sub(beginTime) > table.opts.Precision {
					break collectMetric
				}
				if skip := mfString(tf, visited, fields); skip {
					break collectMetric
				}
			case *iHistogram:
				mfHistogram(tf, fields)
			case *iSummary:
				mfSummary(tf, fields)
			case *iPoint:
				mpP := mpPoint(table, tf)
				mb.Metrics = append(mb.Metrics, mpP)
			default:
				panic(fmt.Sprintf("invalid type '%+v' encountered", v))
			}
			idxInc++
		}

		if len(fields) > 0 {
			mp.Fields = fields
			mb.Metrics = append(mb.Metrics, mp)
		}

		if startIdx += idxInc; startIdx >= len(table.timeFields) {
			break
		}
	}

	// clear cache
	table.timeFields = []*timeField{}
	atomic.StoreInt32(&table.dirty, 0)
}

func newMetricPoint(table *iTable, beginTime time.Time) *metric.MetricPoint {
	pbTimestamp, err := protobuf.TimestampProto(beginTime)
	if err != nil {
		return nil
	}

	mp := &metric.MetricPoint{
		Name: table.name,
		Tags: table.keys,
		When: &api.Timestamp{
			Timestamp: *pbTimestamp,
		},
	}

	return mp
}

func mfCounter(tf *timeField, fields map[string]*metric.Field) {
	c := tf.field.(*iCounter)

	c.nextIdx--
	if c.nextIdx == 0 {
		fields[c.name] = &metric.Field{F: &metric.Field_Int64{Int64: c.value}}
	} else if c.nextIdx < 0 {
		panic(fmt.Sprintf("negative index for counter %+v", c))
	}
}

func mfGauge(tf *timeField, visited map[string]bool, fields map[string]*metric.Field) bool {
	g := tf.field.(*iGauge)

	if visitOk, ok := visited[g.name]; ok && visitOk {
		return true
	}

	if g.nextIdx--; g.nextIdx < 0 {
		panic(fmt.Sprintf("negative index for gauge %+v", g))
	}

	fields[g.name] = &metric.Field{F: &metric.Field_Float64{Float64: g.values[tf.subIdx]}}
	visited[g.name] = true

	// clean up current readings
	if g.nextIdx == 0 {
		g.values = []float64{}
	}

	return false
}

func mfBool(tf *timeField, visited map[string]bool, fields map[string]*metric.Field) bool {
	b := tf.field.(*iBool)

	if visitOk, ok := visited[b.name]; ok && visitOk {
		return true
	}

	if b.nextIdx--; b.nextIdx < 0 {
		panic(fmt.Sprintf("negative index for bool %+v", b))
	}

	fields[b.name] = &metric.Field{F: &metric.Field_Bool{Bool: b.values[tf.subIdx]}}
	visited[b.name] = true

	// clean up current readings
	if b.nextIdx == 0 {
		b.values = []bool{}
	}

	return false
}

func mfString(tf *timeField, visited map[string]bool, fields map[string]*metric.Field) bool {
	s := tf.field.(*iString)

	if visitOk, ok := visited[s.name]; ok && visitOk {
		return true
	}

	if s.nextIdx--; s.nextIdx < 0 {
		panic(fmt.Sprintf("negative index for %+v", s))
	}

	fields[s.name] = &metric.Field{F: &metric.Field_String_{String_: s.values[tf.subIdx]}}
	visited[s.name] = true

	// clean up current readings
	if s.nextIdx == 0 {
		s.values = []string{}
	}

	return false
}

func mfHistogram(tf *timeField, fields map[string]*metric.Field) {
	h := tf.field.(*iHistogram)

	h.nextIdx--
	if h.nextIdx == 0 {
		for key, value := range h.values {
			if !h.dirty[key] {
				continue
			}
			h.dirty[key] = false
			hName := h.name + fmt.Sprintf("_%d", key)
			fields[hName] = &metric.Field{F: &metric.Field_Int64{Int64: value}}
		}
	} else if h.nextIdx < 0 {
		panic(fmt.Sprintf("negative index for histogram %+v", h))
	}
}

func mfSummary(tf *timeField, fields map[string]*metric.Field) {
	s := tf.field.(*iSummary)

	s.nextIdx--
	if s.nextIdx == 0 {
		fields[s.name+"_totalCount"] = &metric.Field{F: &metric.Field_Int64{Int64: s.totalCount}}
		fields[s.name+"_totalValue"] = &metric.Field{F: &metric.Field_Float64{Float64: s.totalValue}}
	} else if s.nextIdx < 0 {
		panic(fmt.Sprintf("going negative index for summary %+v", s))
	}
}

func mpPoint(table *iTable, tf *timeField) *metric.MetricPoint {
	pbTimestamp, err := protobuf.TimestampProto(tf.ts)
	if err != nil {
		return nil
	}

	p := tf.field.(*iPoint)

	mfs := make(map[string]*metric.Field)
	for key, field := range p.fields {
		switch field.(type) {
		case int64:
			v := field.(int64)
			mfs[key] = &metric.Field{F: &metric.Field_Int64{Int64: v}}
		case float64:
			v := field.(float64)
			mfs[key] = &metric.Field{F: &metric.Field_Float64{Float64: v}}
		case string:
			v := field.(string)
			mfs[key] = &metric.Field{F: &metric.Field_String_{String_: v}}
		case bool:
			v := field.(bool)
			mfs[key] = &metric.Field{F: &metric.Field_Bool{Bool: v}}
		}
	}

	mp := &metric.MetricPoint{
		Name:   table.name,
		Tags:   p.keys,
		Fields: mfs,
		When: &api.Timestamp{
			Timestamp: *pbTimestamp,
		},
	}

	return mp
}
