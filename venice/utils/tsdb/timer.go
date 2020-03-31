package tsdb

import (
	"context"
	"fmt"
	"sync/atomic"
	"time"

	"github.com/pensando/sw/venice/globals"

	protobuf "github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/citadel/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/tsdb/metrics"
)

// establishConn connects to citadel instances over grpc and load balancer
// it would forever keep trying to connect to citadel unless parent context is canceled
func establishConn() bool {
	for {
		var err error
		rpckitOpts := make([]rpckit.Option, 0)
		if global.opts.ResolverClient != nil {
			rpckitOpts = append(rpckitOpts, rpckit.WithBalancer(balancer.New(global.opts.ResolverClient)))
		}
		rpckitOpts = append(rpckitOpts, rpckit.WithLoggerEnabled(false))

		if global.opts.Collector != "" {
			if global.rpcClient != nil {
				global.rpcClient.Close()
				global.rpcClient = nil
			}
			global.rpcClient, err = rpckit.NewRPCClient(global.opts.ClientName, global.opts.Collector, rpckitOpts...)
			if err == nil {
				break
			}
			log.Errorf("[%s]error establishing the connection to %s, %s", global.opts.ClientName, global.opts.Collector, err)
		}

		select {
		case <-time.After(global.opts.ConnectionRetryInterval):
		case <-global.context.Done():
			log.Infof("aborting tsdb transmit thread: parent context ended")
			return false
		}
	}

	global.mc = metric.NewMetricApiClient(global.rpcClient.ClientConn)
	log.Infof("connected to %+v, metrics limit %v", global.opts.Collector, maxMetricsPoints)
	return true
}

// periodicTransmit is responsible for calling the timer based send of all metrics
// most of the metrics (except precision metrics) are timestamped at the push
// intervals; this keeps the collection and distribution overhead lower
func periodicTransmit() {
	defer global.wg.Done()
	for {
		if global.mc == nil && !establishConn() {
			return
		}
		select {
		case <-time.After(global.opts.SendInterval):
			err := sendAllObjs(global.context)
			if err != nil {
				// Force a reconnection
				global.mc = nil
				log.Infof("tsdb lost connection to %v", globals.Collector)
			}
		case <-global.context.Done():
			// flush any metrics that are not yet pushed out
			ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(time.Second))
			defer cancel()
			sendAllObjs(ctx)
			log.Infof("aborting tsdb transmit thread: parent context ended")
			return
		}
	}
}

// Push allows user to initiate an immediate push for all the points
// in a given object
func (obj *iObj) Push() {
	dbName := global.opts.DBName

	// create a metric bundle
	mb := &metric.MetricBundle{DbName: dbName, Reporter: global.opts.ClientName, Metrics: nil}
	obj.Lock()
	obj.getMetricBundles(mb)
	obj.dirty = false
	obj.Unlock()

	// push metrics to collector
	if len(mb.Metrics) > 0 {
		_, err := global.mc.WriteMetrics(global.context, mb)
		if err != nil {
			log.Errorf("sendPoints : WriteMetrics failed with err: %s", err)
		}
	}
}

// sendAllObjs collects all objects/metrics collected since last timer expiry
// it looks for dirty objects (objects that have changed) and deleted objects
// it creates metric bundles to and use collector library to push metrics to the
// collector
func sendAllObjs(ctx context.Context) error {
	objs := []*iObj{}

	// fetch dirty objs
	global.Lock()
	dbName := global.opts.DBName
	for _, obj := range global.objs {
		if !obj.opts.Local && obj.dirty {
			objs = append(objs, obj)
		}
	}
	for _, obj := range global.deletedObjs {
		if !obj.opts.Local && obj.dirty {
			objs = append(objs, obj)
		}
	}
	global.deletedObjs = []*iObj{}
	global.Unlock()

	// create metric bundle
	mb := &metric.MetricBundle{DbName: dbName, Reporter: global.opts.ClientName, Metrics: nil}
	for _, obj := range objs {
		obj.Lock()
		obj.getMetricBundles(mb)
		obj.Unlock()
	}

	// push metrics to collector
	if len(mb.Metrics) > 0 {
		_, err := global.mc.WriteMetrics(ctx, mb)
		if err != nil {
			log.Errorf("sendPoints : WriteMetrics failed with err: %s", err)
			global.sendErrors++
			return err
		}
		atomic.AddUint64(&global.numPoints, uint64(len(mb.Metrics)))
	}
	return nil
}

// getMetricBundles would fetch all metrics from an object and save them in
// the provided metric bundle. It avoids creating points for atomic transactions
// given that the timer can fire up in between updates of metrics
func (obj *iObj) getMetricBundles(mb *metric.MetricBundle) {
	// latest snapshot is not collected if 1) object is atomic 2) it is set with its own collection interval
	if !obj.atomic && obj.collectionTicks == 0 {
		if obj.ts.IsZero() {
			obj.ts = time.Now()
		}
		createNewMetricPoint(obj, time.Time{})
	}

	if len(obj.metricPoints) > 0 {
		mb.Metrics = append(mb.Metrics, obj.metricPoints...)
		obj.metricPoints = []*metric.MetricPoint{}
	}

	// clear cache if there is no active atomic transaction
	if !obj.atomic {
		obj.dirty = false
		obj.ts = time.Time{}
	}
}

// createNewMetricPoint determines if we need to create a new
// point for a given obj; if it does, it constructs a new metric-point
// and adds it to the list of metric-points for the obj
// This function is called within obj lock boundaries
func createNewMetricPoint(obj *iObj, ts time.Time) {
	// new metric bundle is not neededis if any of the following is true:
	// - obj is undergoing an atomic transaction, then we can add all fields as part of a point
	// - current timestamp for the obj is zero then the point is empty and can be filled in by the field
	// - if timestamp is same as specified by the caller, then two fields can be part of the same point
	if obj.atomic || obj.ts.IsZero() || obj.ts == ts {
		return
	}

	// limit number of points stored in TSDB client
	if len(obj.metricPoints) >= maxMetricsPoints {
		atomic.AddUint64(&global.ignoredPoints, 1)
		return
	}

	pbTimestamp, err := protobuf.TimestampProto(obj.ts)
	if err != nil {
		log.Errorf("unable to convert timestamp")
		return
	}

	mfs := make(map[string]*metric.Field)
	for key, field := range obj.fields {
		switch k := field.(type) {
		case *iCounter:
			c := field.(*iCounter)
			mfs[key] = &metric.Field{F: &metric.Field_Int64{Int64: c.value}}
		case *iGauge:
			g := field.(*iGauge)
			mfs[key] = &metric.Field{F: &metric.Field_Float64{Float64: g.value}}
		case *iPrecisionGauge:
			g := field.(*iPrecisionGauge)
			mfs[key] = &metric.Field{F: &metric.Field_Float64{Float64: g.value}}
		case *iBool:
			b := field.(*iBool)
			mfs[key] = &metric.Field{F: &metric.Field_Bool{Bool: b.value}}
		case *iString:
			s := field.(*iString)
			mfs[key] = &metric.Field{F: &metric.Field_String_{String_: s.value}}
		case *iHistogram:
			h := field.(*iHistogram)
			for hKey, hValue := range h.values {
				mfKey := h.name + fmt.Sprintf("_%d", hKey)
				mfs[mfKey] = &metric.Field{F: &metric.Field_Int64{Int64: hValue}}
			}
		case *iSummary:
			s := field.(*iSummary)
			mfs[s.name+"_totalCount"] = &metric.Field{F: &metric.Field_Int64{Int64: s.totalCount}}
			mfs[s.name+"_totalValue"] = &metric.Field{F: &metric.Field_Float64{Float64: s.totalValue}}
		default:
			panic(fmt.Sprintf("unrecognized type %T", k))

		}
	}

	if len(mfs) == 0 {
		return
	}

	mp := &metric.MetricPoint{
		Name:   obj.tableName,
		Tags:   obj.keys,
		Fields: mfs,
		When: &api.Timestamp{
			Timestamp: *pbTimestamp,
		},
	}

	obj.metricPoints = append(obj.metricPoints, mp)

	// clear the current point's timestamp
	obj.ts = time.Time{}
}

// createNewMetricPointFromKeysFields is the most primitive function to
// create a point in a time series, it takes keys and fields directly
// from the user to create a metric point for a specified obj name
func createNewMetricPointFromKeysFields(obj *iObj, keys map[string]string, fields map[string]interface{}, ts time.Time) {
	pbTimestamp, err := protobuf.TimestampProto(ts)
	if err != nil {
		log.Errorf("unable to convert timestamp")
		return
	}

	// limit number of points stored in TSDB client
	obj.Lock()
	if len(obj.metricPoints) >= maxMetricsPoints {
		obj.Unlock()
		atomic.AddUint64(&global.ignoredPoints, 1)
		return
	}
	obj.Unlock()

	mfs := make(map[string]*metric.Field)
	for key, field := range fields {
		switch k := field.(type) {
		case int64:
			v := field.(int64)
			mfs[key] = &metric.Field{F: &metric.Field_Int64{Int64: v}}
		case uint64:
			v := field.(uint64)
			mfs[key] = &metric.Field{F: &metric.Field_Uint64{Uint64: v}}
		case float64:
			v := field.(float64)
			mfs[key] = &metric.Field{F: &metric.Field_Float64{Float64: v}}
		case string:
			v := field.(string)
			mfs[key] = &metric.Field{F: &metric.Field_String_{String_: v}}
		case bool:
			v := field.(bool)
			mfs[key] = &metric.Field{F: &metric.Field_Bool{Bool: v}}
		case metrics.Counter:
			v := uint64(field.(metrics.Counter))
			mfs[key] = &metric.Field{F: &metric.Field_Uint64{Uint64: v}}
		case metrics.Gauge:
			v := float64(field.(metrics.Gauge))
			mfs[key] = &metric.Field{F: &metric.Field_Float64{Float64: v}}
		default:
			log.Errorf("Unrecognized type %T", k)
		}
	}

	if len(mfs) == 0 {
		log.Errorf("Bad MP -- no fields")
		return
	}

	tags := make(map[string]string)
	for k, v := range keys {
		tags[k] = v
	}

	obj.Lock()
	defer obj.Unlock()

	mp := &metric.MetricPoint{
		Name:   obj.tableName,
		Tags:   tags,
		Fields: mfs,
		When: &api.Timestamp{
			Timestamp: *pbTimestamp,
		},
	}

	obj.metricPoints = append(obj.metricPoints, mp)
}

// Debug returns tsdb client info
func Debug() interface{} {
	if global == nil {
		return map[string]uint64{}
	}

	return map[string]uint64{
		"numPointsReported": atomic.LoadUint64(&global.numPoints),
		"numSendErrors":     atomic.LoadUint64(&global.sendErrors),
		"numIgnoredPoints":  atomic.LoadUint64(&global.ignoredPoints)}
}

// collectionTimer is responsible for generating points from objects that
// haven't posted a point
func collectionTimer() {
	defer global.wg.Done()
	for {
		select {
		case <-time.After(minCollectionInterval):
			generateCollectionIntervalMetrics()
		case <-global.context.Done():
			log.Infof("aborting tsdb collection thread: parent context ended")
			return
		}
	}
}

// generateCollectionIntervalMetrics generates the metrics points
// within collection interval and save the metrics within the object itself
func generateCollectionIntervalMetrics() {
	objs := []*iObj{}

	// collect objects that requie collection
	global.Lock()
	for _, obj := range global.objs {
		if !obj.atomic && !obj.opts.Local && obj.opts.collectionTicks > 0 {
			objs = append(objs, obj)
		}
	}
	global.Unlock()

	for _, obj := range objs {
		obj.Lock()

		// adjust the ticks, if zero add metrics point
		obj.collectionTicks--
		if obj.collectionTicks > 0 {
			continue
		}
		obj.collectionTicks = obj.opts.collectionTicks

		// create a point for the object
		if obj.ts.IsZero() {
			obj.ts = time.Now()
		}
		obj.dirty = true
		createNewMetricPoint(obj, time.Time{})
		obj.Unlock()
	}
}
