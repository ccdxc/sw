// Package cachedb provides a cache-only TSDB
package cachedb

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"regexp"
	"runtime"
	"sync"
	"time"

	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/pkg/estimator"
	"github.com/influxdata/influxdb/query"
	"github.com/influxdata/influxdb/tsdb"
	// initialize index
	_ "github.com/influxdata/influxdb/tsdb/index"
	"github.com/influxdata/influxdb/tsdb/index/inmem"
	"github.com/influxdata/influxql"
	"go.uber.org/zap"

	"github.com/pensando/sw/venice/utils/log"
)

//go:generate tmpl -data=@iterator.gen.go.tmpldata iterator.gen.go.tmpl
//generate tmpl -data=@file_store.gen.go.tmpldata file_store.gen.go.tmpl
//go:generate tmpl -data=@encoding.gen.go.tmpldata encoding.gen.go.tmpl
//generate tmpl -data=@compact.gen.go.tmpldata compact.gen.go.tmpl
//go:generate tmpl -data=@values_test.go.tmpldata values_test.go.tmpl
//go:generate tmpl -data=@iterator.gen_test.go.tmpldata iterator.gen_test.go.tmpl

//Note: This is a cache-only implementation of Engine. Therefore,
// methods applicable to persistent storage are stubbed out.
func init() {
	tsdb.RegisterEngine("cachedb", NewCacheEngine)
}

var (
	// Ensure Engine implements the interface.
	_                      tsdb.Engine = &CacheEngine{}
	nullKeyCursor                      = &KeyCursor{}
	timeBytes                          = []byte("time")
	keyFieldSeparatorBytes             = []byte(keyFieldSeparator)
)

const (
	// keyFieldSeparator separates the series key from the field name in the composite key
	// that identifies a specific field in series
	keyFieldSeparator = "#!~#"
)

// CacheEngine represents a storage engine with compressed blocks.
type CacheEngine struct {
	mu sync.RWMutex

	id       uint64
	database string
	path     string

	index    tsdb.Index
	fieldset *tsdb.MeasurementFieldSet

	Cache *Cache

	MaxPointsPerBlock int

	// CacheFlushMemorySizeThreshold specifies the minimum size threshold for
	// the cache when the engine should write a snapshot to a TSM file
	CacheFlushMemorySizeThreshold uint64

	// CacheFlushWriteColdDuration specifies the length of time after which if
	// no writes have been committed to the WAL, the engine will write
	// a snapshot of the cache to a TSM file
	CacheFlushWriteColdDuration time.Duration
}

// NewCacheEngine returns a new instance of CacheEngine.
func NewCacheEngine(id uint64, idx tsdb.Index, database, path string, walPath string, opt tsdb.EngineOptions) tsdb.Engine {
	cache := NewCache(uint64(opt.Config.CacheMaxMemorySize), path)
	e := &CacheEngine{
		id:       id,
		database: database,
		path:     path,
		index:    idx,

		Cache: cache,

		CacheFlushWriteColdDuration: time.Duration(opt.Config.CacheSnapshotWriteColdDuration),
	}

	// Attach fieldset to index.
	e.index.SetFieldSet(e.fieldset)

	return e
}

// SetEnabled sets whether the engine is enabled.
func (e *CacheEngine) SetEnabled(enabled bool) {
}

// SetCompactionsEnabled enables compactions on the engine.  When disabled
// all running compactions are aborted and new compactions stop running.
func (e *CacheEngine) SetCompactionsEnabled(enabled bool) {
}

// ScheduleFullCompaction is not implemented
func (e *CacheEngine) ScheduleFullCompaction() error {
	return nil
}

// Path returns the path the engine was opened with.
func (e *CacheEngine) Path() string { return e.path }

// SetFieldName sets the fieldname in the index
func (e *CacheEngine) SetFieldName(measurement []byte, name string) {
	e.index.SetFieldName(measurement, name)
}

// MeasurementExists checks if the meas exists
func (e *CacheEngine) MeasurementExists(name []byte) (bool, error) {
	return e.index.MeasurementExists(name)
}

// MeasurementNamesByExpr returns the measurements that match expr
func (e *CacheEngine) MeasurementNamesByExpr(expr influxql.Expr) ([][]byte, error) {
	return e.index.MeasurementNamesByExpr(expr)
}

// MeasurementNamesByRegex returns the measurements that match re
func (e *CacheEngine) MeasurementNamesByRegex(re *regexp.Regexp) ([][]byte, error) {
	return e.index.MeasurementNamesByRegex(re)
}

// MeasurementFields returns the measurement fields for a measurement.
func (e *CacheEngine) MeasurementFields(measurement []byte) *tsdb.MeasurementFields {
	return e.fieldset.CreateFieldsIfNotExists(measurement)
}

// MeasurementFieldSet fetches the fieldset
func (e *CacheEngine) MeasurementFieldSet() *tsdb.MeasurementFieldSet {
	return e.fieldset
}

// HasTagKey checks if the key exists in the measurement
func (e *CacheEngine) HasTagKey(name, key []byte) (bool, error) {
	return e.index.HasTagKey(name, key)
}

// MeasurementTagKeysByExpr finds matching keys.
func (e *CacheEngine) MeasurementTagKeysByExpr(name []byte, expr influxql.Expr) (map[string]struct{}, error) {
	return e.index.MeasurementTagKeysByExpr(name, expr)
}

// TagKeyHasAuthorizedSeries is n/a
func (e *CacheEngine) TagKeyHasAuthorizedSeries(auth query.Authorizer, name []byte, key string) bool {
	return false
}

// MeasurementTagKeyValuesByExpr returns a set of tag values filtered by an expression.
//
// MeasurementTagKeyValuesByExpr relies on the provided tag keys being sorted.
// The caller can indicate the tag keys have been sorted by setting the
// keysSorted argument appropriately. Tag values are returned in a slice that
// is indexible according to the sorted order of the tag keys, e.g., the values
// for the earliest tag k will be available in index 0 of the returned values
// slice.
//
func (e *CacheEngine) MeasurementTagKeyValuesByExpr(auth query.Authorizer, name []byte, keys []string, expr influxql.Expr, keysSorted bool) ([][]string, error) {
	return e.index.MeasurementTagKeyValuesByExpr(auth, name, keys, expr, keysSorted)
}

// ForEachMeasurementTagKey iterates over the tagkeys
func (e *CacheEngine) ForEachMeasurementTagKey(name []byte, fn func(key []byte) error) error {
	return e.index.ForEachMeasurementTagKey(name, fn)
}

// TagKeyCardinality returns the number of series for the key
func (e *CacheEngine) TagKeyCardinality(name, key []byte) int {
	return e.index.TagKeyCardinality(name, key)
}

// SeriesN returns the unique number of series in the index.
func (e *CacheEngine) SeriesN() int64 {
	return e.index.SeriesN()
}

// SeriesSketches is unused
func (e *CacheEngine) SeriesSketches() (estimator.Sketch, estimator.Sketch, error) {
	return e.index.SeriesSketches()
}

// MeasurementsSketches is unused in cacheengine
func (e *CacheEngine) MeasurementsSketches() (estimator.Sketch, estimator.Sketch, error) {
	return e.index.MeasurementsSketches()
}

// LastModified returns the time when this shard was last modified.
func (e *CacheEngine) LastModified() time.Time {
	return e.Cache.LastModified()
}

// Statistics returns statistics for periodic monitoring.
func (e *CacheEngine) Statistics(tags map[string]string) []models.Statistic {
	statistics := make([]models.Statistic, 0, 4)
	return statistics
}

// DiskSize returns the total size in bytes of all TSM and WAL segments on disk.
func (e *CacheEngine) DiskSize() int64 {
	return 0
}

// Open opens and initializes the engine.
func (e *CacheEngine) Open() error {
	fields := tsdb.NewMeasurementFieldSet()
	e.fieldset = fields
	e.index.SetFieldSet(fields)
	return nil
}

// Close closes the engine. Subsequent calls to Close are a nop.
func (e *CacheEngine) Close() error {
	return nil
}

// WithLogger sets the logger for the engine.
func (e *CacheEngine) WithLogger(log *zap.Logger) {
}

// LoadMetadataIndex loads the shard metadata into memory.
func (e *CacheEngine) LoadMetadataIndex(shardID uint64, index tsdb.Index) error {
	now := time.Now()

	// Save reference to index for iterator creation.
	e.index = index

	// load metadata from the Cache
	if err := e.Cache.ApplyEntryFn(func(key []byte, entry *entry) error {
		fieldType, err := entry.values.InfluxQLType()
		if err != nil {
			log.Info(fmt.Sprintf("error getting the data type of values for key %s: %s", key, err.Error()))
		}

		err = e.addToIndexFromKey(key, fieldType)
		return err
	}); err != nil {
		return err
	}

	log.Info(fmt.Sprintf("Meta data index for shard %d loaded in %v", shardID, time.Since(now)))
	return nil
}

// IsIdle returns true if the cache is empty, there are no running compactions and the
// shard is fully compacted.
func (e *CacheEngine) IsIdle() bool {
	cacheEmpty := e.Cache.Size() == 0

	return cacheEmpty
}

// Free releases any resources held by the engine to free up memory or CPU.
func (e *CacheEngine) Free() error {
	e.Cache.Free()
	return nil
}

// Backup is n/a for cachedb
func (e *CacheEngine) Backup(w io.Writer, basePath string, since time.Time) error {
	return nil
}

// Restore is n/a for cachedb
func (e *CacheEngine) Restore(r io.Reader, basePath string) error {
	return nil
}

// Import is n/a for cachedb
func (e *CacheEngine) Import(r io.Reader, basePath string) error {
	return nil
}

// addToIndexFromKey will pull the measurement name, series key, and field name from a composite key and add it to the
// database index and measurement fields
func (e *CacheEngine) addToIndexFromKey(key []byte, fieldType influxql.DataType) error {
	seriesKey, field := SeriesAndFieldFromCompositeKey(key)
	name := tsdb.MeasurementFromSeriesKey(seriesKey)

	mf := e.fieldset.CreateFieldsIfNotExists(name)
	if err := mf.CreateFieldIfNotExists(field, fieldType, false); err != nil {
		return err
	}

	// Build in-memory index, if necessary.
	if e.index.Type() == inmem.IndexName {
		tags, _ := models.ParseTags(seriesKey)
		if err := e.index.InitializeSeries(seriesKey, name, tags); err != nil {
			return err
		}
	}

	return nil
}

// WritePoints writes metadata and point data into the engine.
// It returns an error if new points are added to an existing key.
func (e *CacheEngine) WritePoints(points []models.Point) error {
	values := make(map[string][]Value, len(points))
	var keyBuf []byte
	var baseLen int
	for _, p := range points {
		keyBuf = append(keyBuf[:0], p.Key()...)
		keyBuf = append(keyBuf, keyFieldSeparator...)
		baseLen = len(keyBuf)
		iter := p.FieldIterator()
		t := p.Time().UnixNano()
		for iter.Next() {
			// Skip fields name "time", they are illegal
			if bytes.Equal(iter.FieldKey(), timeBytes) {
				continue
			}

			keyBuf = append(keyBuf[:baseLen], iter.FieldKey()...)
			var v Value
			switch iter.Type() {
			case models.Float:
				fv, err := iter.FloatValue()
				if err != nil {
					return err
				}
				v = NewFloatValue(t, fv)
			case models.Integer:
				iv, err := iter.IntegerValue()
				if err != nil {
					return err
				}
				v = NewIntegerValue(t, iv)
			case models.Unsigned:
				iv, err := iter.UnsignedValue()
				if err != nil {
					return err
				}
				v = NewUnsignedValue(t, iv)
			case models.String:
				v = NewStringValue(t, iter.StringValue())
			case models.Boolean:
				bv, err := iter.BooleanValue()
				if err != nil {
					return err
				}
				v = NewBooleanValue(t, bv)
			default:
				return fmt.Errorf("unknown field type for %s: %s", string(iter.FieldKey()), p.String())
			}
			values[string(keyBuf)] = append(values[string(keyBuf)], v)
		}
	}

	e.mu.RLock()
	defer e.mu.RUnlock()

	// first try to write to the cache
	err := e.Cache.WriteMulti(values)
	if err != nil {
		return err
	}

	return nil
}

// KeyMap creates a map of series keys
func (e *CacheEngine) KeyMap() map[string]bool {
	km := map[string]bool{}

	for _, k := range e.Cache.unsortedKeys() {
		seriesKey, _ := SeriesAndFieldFromCompositeKey([]byte(k))
		km[string(seriesKey)] = true
	}

	return km
}

// deleteSeries removes all series keys from the engine.
func (e *CacheEngine) deleteSeries(seriesKeys [][]byte) error {
	return nil
}

// DeleteSeriesRange is n/a for cachedb
func (e *CacheEngine) DeleteSeriesRange(tsi tsdb.SeriesIterator, min, max int64) error {
	return nil
}

// DeleteMeasurement deletes a measurement and all related series.
func (e *CacheEngine) DeleteMeasurement(name []byte) error {
	return nil
}

// MeasurementSeriesKeysByExprIterator finds matching keys for the expr
func (e *CacheEngine) MeasurementSeriesKeysByExprIterator(name []byte, expr influxql.Expr) (tsdb.SeriesIterator, error) {
	return e.index.MeasurementSeriesKeysByExprIterator(name, expr)
}

// ForEachMeasurementName iterates over each measurement name in the engine.
func (e *CacheEngine) ForEachMeasurementName(fn func(name []byte) error) error {
	return e.index.ForEachMeasurementName(fn)
}

// MeasurementSeriesKeysByExpr returns a list of series keys matching expr.
func (e *CacheEngine) MeasurementSeriesKeysByExpr(name []byte, expr influxql.Expr) ([][]byte, error) {
	return e.index.MeasurementSeriesKeysByExpr(name, expr)
}

// CreateSeriesListIfNotExists creates the series in the index table
func (e *CacheEngine) CreateSeriesListIfNotExists(keys, names [][]byte, tagsSlice []models.Tags) error {
	return e.index.CreateSeriesListIfNotExists(keys, names, tagsSlice)
}

// CreateSeriesIfNotExists creates the series in the index table
func (e *CacheEngine) CreateSeriesIfNotExists(key, name []byte, tags models.Tags) error {
	return e.index.CreateSeriesIfNotExists(key, name, tags)
}

// WriteTo is not implemented.
func (e *CacheEngine) WriteTo(w io.Writer) (n int64, err error) { panic("not implemented") }

// WriteSnapshot will snapshot the cache and write a new TSM file with its contents, releasing the snapshot when done.
func (e *CacheEngine) WriteSnapshot() error {
	return nil
}

// CreateSnapshot will create a temp directory that holds
// temporary hardlinks to the underylyng shard files.
func (e *CacheEngine) CreateSnapshot() (string, error) {
	return "", nil
}

// ShouldCompactCache returns true if the Cache is over its flush threshold
// or if the passed in lastWriteTime is older than the write cold threshold.
func (e *CacheEngine) ShouldCompactCache(lastWriteTime time.Time) bool {
	return false
}

// KeyCursor returns a KeyCursor for the given key starting at time t.
func (e *CacheEngine) KeyCursor(key []byte, t int64, ascending bool) *KeyCursor {
	return nullKeyCursor // return a null cursor as we have no storage
}

// CreateIterator returns an iterator for the measurement based on opt.
func (e *CacheEngine) CreateIterator(context context.Context, measurement string, opt query.IteratorOptions) (query.Iterator, error) {
	if call, ok := opt.Expr.(*influxql.Call); ok {
		if opt.Interval.IsZero() {
			if call.Name == "first" || call.Name == "last" {
				refOpt := opt
				refOpt.Limit = 1
				refOpt.Ascending = call.Name == "first"
				refOpt.Ordered = true
				refOpt.Expr = call.Args[0]

				itrs, err := e.createVarRefIterator(measurement, refOpt)
				if err != nil {
					return nil, err
				}
				return newMergeFinalizerIterator(context, itrs, opt)
			}
		}

		inputs, err := e.createCallIterator(measurement, call, opt)
		if err != nil {
			return nil, err
		} else if len(inputs) == 0 {
			return nil, nil
		}
		return newMergeFinalizerIterator(context, inputs, opt)
	}

	itrs, err := e.createVarRefIterator(measurement, opt)
	if err != nil {
		return nil, err
	}
	return newMergeFinalizerIterator(context, itrs, opt)
}

func (e *CacheEngine) createCallIterator(measurement string, call *influxql.Call, opt query.IteratorOptions) ([]query.Iterator, error) {
	ref, _ := call.Args[0].(*influxql.VarRef)

	if exists, err := e.index.MeasurementExists([]byte(measurement)); err != nil {
		return nil, err
	} else if !exists {
		return nil, nil
	}

	// Determine tagsets for this measurement based on dimensions and filters.
	tagSets, err := e.index.TagSets([]byte(measurement), opt)
	if err != nil {
		return nil, err
	}

	// Reverse the tag sets if we are ordering by descending.
	if !opt.Ascending {
		for _, t := range tagSets {
			t.Reverse()
		}
	}

	// Calculate tag sets and apply SLIMIT/SOFFSET.
	tagSets = query.LimitTagSets(tagSets, opt.SLimit, opt.SOffset)

	itrs := make([]query.Iterator, 0, len(tagSets))
	if err := func() error {
		for _, t := range tagSets {
			// Abort if the query was killed
			select {
			case <-opt.InterruptCh:
				query.Iterators(itrs).Close()
				return err
			default:
			}

			inputs, err := e.createTagSetIterators(ref, measurement, t, opt)
			if err != nil {
				return err
			} else if len(inputs) == 0 {
				continue
			}

			// Wrap each series in a call iterator.
			for i, input := range inputs {
				if opt.InterruptCh != nil {
					input = query.NewInterruptIterator(input, opt.InterruptCh)
				}

				itr, err := query.NewCallIterator(input, opt)
				if err != nil {
					query.Iterators(inputs).Close()
					return err
				}
				inputs[i] = itr
			}

			itr := query.NewParallelMergeIterator(inputs, opt, runtime.GOMAXPROCS(0))
			itrs = append(itrs, itr)
		}
		return nil
	}(); err != nil {
		query.Iterators(itrs).Close()
		return nil, err
	}

	return itrs, nil
}

// createVarRefIterator creates an iterator for a variable reference.
func (e *CacheEngine) createVarRefIterator(measurement string, opt query.IteratorOptions) ([]query.Iterator, error) {
	ref, _ := opt.Expr.(*influxql.VarRef)

	if exists, err := e.index.MeasurementExists([]byte(measurement)); err != nil {
		return nil, err
	} else if !exists {
		return nil, nil
	}

	// Determine tagsets for this measurement based on dimensions and filters.
	tagSets, err := e.index.TagSets([]byte(measurement), opt)
	if err != nil {
		return nil, err
	}

	// Reverse the tag sets if we are ordering by descending.
	if !opt.Ascending {
		for _, t := range tagSets {
			t.Reverse()
		}
	}

	// Calculate tag sets and apply SLIMIT/SOFFSET.
	tagSets = query.LimitTagSets(tagSets, opt.SLimit, opt.SOffset)

	itrs := make([]query.Iterator, 0, len(tagSets))
	if err := func() error {
		for _, t := range tagSets {
			inputs, err := e.createTagSetIterators(ref, measurement, t, opt)
			if err != nil {
				return err
			} else if len(inputs) == 0 {
				continue
			}

			// If we have a LIMIT or OFFSET and the grouping of the outer query
			// is different than the current grouping, we need to perform the
			// limit on each of the individual series keys instead to improve
			// performance.
			if (opt.Limit > 0 || opt.Offset > 0) && len(opt.Dimensions) != len(opt.GroupBy) {
				for i, input := range inputs {
					inputs[i] = newLimitIterator(input, opt)
				}
			}

			itr, err := query.Iterators(inputs).Merge(opt)
			if err != nil {
				query.Iterators(inputs).Close()
				return err
			}

			// Apply a limit on the merged iterator.
			if opt.Limit > 0 || opt.Offset > 0 {
				if len(opt.Dimensions) == len(opt.GroupBy) {
					// When the final dimensions and the current grouping are
					// the same, we will only produce one series so we can use
					// the faster limit iterator.
					itr = newLimitIterator(itr, opt)
				} else {
					// When the dimensions are different than the current
					// grouping, we need to account for the possibility there
					// will be multiple series. The limit iterator in the
					// influxql package handles that scenario.
					itr = query.NewLimitIterator(itr, opt)
				}
			}
			itrs = append(itrs, itr)
		}
		return nil
	}(); err != nil {
		query.Iterators(itrs).Close()
		return nil, err
	}

	return itrs, nil
}

// createTagSetIterators creates a set of iterators for a tagset.
func (e *CacheEngine) createTagSetIterators(ref *influxql.VarRef, name string, t *query.TagSet, opt query.IteratorOptions) ([]query.Iterator, error) {
	// Set parallelism by number of logical cpus.
	parallelism := runtime.GOMAXPROCS(0)
	if parallelism > len(t.SeriesKeys) {
		parallelism = len(t.SeriesKeys)
	}

	// Create series key groupings w/ return error.
	groups := make([]struct {
		keys    []string
		filters []influxql.Expr
		itrs    []query.Iterator
		err     error
	}, parallelism)

	// Group series keys.
	n := len(t.SeriesKeys) / parallelism
	for i := 0; i < parallelism; i++ {
		group := &groups[i]

		if i < parallelism-1 {
			group.keys = t.SeriesKeys[i*n : (i+1)*n]
			group.filters = t.Filters[i*n : (i+1)*n]
		} else {
			group.keys = t.SeriesKeys[i*n:]
			group.filters = t.Filters[i*n:]
		}
	}

	// Read series groups in parallel.
	var wg sync.WaitGroup
	for i := range groups {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			groups[i].itrs, groups[i].err = e.createTagSetGroupIterators(ref, name, groups[i].keys, t, groups[i].filters, opt)
		}(i)
	}
	wg.Wait()

	// Determine total number of iterators so we can allocate only once.
	var itrN int
	for _, group := range groups {
		itrN += len(group.itrs)
	}

	// Combine all iterators together and check for errors.
	var err error
	itrs := make([]query.Iterator, 0, itrN)
	for _, group := range groups {
		if group.err != nil {
			err = group.err
		}
		itrs = append(itrs, group.itrs...)
	}

	// If an error occurred, make sure we close all created iterators.
	if err != nil {
		query.Iterators(itrs).Close()
		return nil, err
	}

	return itrs, nil
}

// createTagSetGroupIterators creates a set of iterators for a subset of a tagset's series.
func (e *CacheEngine) createTagSetGroupIterators(ref *influxql.VarRef, name string, seriesKeys []string, t *query.TagSet, filters []influxql.Expr, opt query.IteratorOptions) ([]query.Iterator, error) {
	itrs := make([]query.Iterator, 0, len(seriesKeys))
	for i, seriesKey := range seriesKeys {
		var conditionFields []influxql.VarRef
		if filters[i] != nil {
			// Retrieve non-time fields from this series filter and filter out tags.
			conditionFields = influxql.ExprNames(filters[i])
		}

		itr, err := e.createVarRefSeriesIterator(ref, name, seriesKey, t, filters[i], conditionFields, opt)
		if err != nil {
			return itrs, err
		} else if itr == nil {
			continue
		}
		itrs = append(itrs, itr)

		// Abort if the query was killed
		select {
		case <-opt.InterruptCh:
			query.Iterators(itrs).Close()
			return nil, err
		default:
		}

		// Enforce series limit at creation time.
		if opt.MaxSeriesN > 0 && len(itrs) > opt.MaxSeriesN {
			query.Iterators(itrs).Close()
			return nil, fmt.Errorf("max-select-series limit exceeded: (%d/%d)", len(itrs), opt.MaxSeriesN)
		}

	}
	return itrs, nil
}

// createVarRefSeriesIterator creates an iterator for a variable reference for a series.
func (e *CacheEngine) createVarRefSeriesIterator(ref *influxql.VarRef, name string, seriesKey string, t *query.TagSet, filter influxql.Expr, conditionFields []influxql.VarRef, opt query.IteratorOptions) (query.Iterator, error) {
	_, tfs := models.ParseKey([]byte(seriesKey))
	tags := query.NewTags(tfs.Map())

	// Create options specific for this series.
	itrOpt := opt
	itrOpt.Condition = filter

	// Build auxilary cursors.
	// Tag values should be returned if the field doesn't exist.
	var aux []cursorAt
	if len(opt.Aux) > 0 {
		aux = make([]cursorAt, len(opt.Aux))
		for i, ref := range opt.Aux {
			// Create cursor from field if a tag wasn't requested.
			if ref.Type != influxql.Tag {
				cur := e.buildCursor(name, seriesKey, tfs, &ref, opt)
				if cur != nil {
					aux[i] = newBufCursor(cur, opt.Ascending)
					continue
				}

				// If a field was requested, use a nil cursor of the requested type.
				switch ref.Type {
				case influxql.Float, influxql.AnyField:
					aux[i] = nilFloatLiteralValueCursor
					continue
				case influxql.Integer:
					aux[i] = nilIntegerLiteralValueCursor
					continue
				case influxql.Unsigned:
					aux[i] = nilUnsignedLiteralValueCursor
					continue
				case influxql.String:
					aux[i] = nilStringLiteralValueCursor
					continue
				case influxql.Boolean:
					aux[i] = nilBooleanLiteralValueCursor
					continue
				}
			}

			// If field doesn't exist, use the tag value.
			if v := tags.Value(ref.Val); v == "" {
				// However, if the tag value is blank then return a null.
				aux[i] = nilStringLiteralValueCursor
			} else {
				aux[i] = &literalValueCursor{value: v}
			}
		}
	}

	// Remove _tagKey condition field.
	// We can't seach on it because we can't join it to _tagValue based on time.
	if varRefSliceContains(conditionFields, "_tagKey") {
		conditionFields = varRefSliceRemove(conditionFields, "_tagKey")

		// Remove _tagKey conditional references from iterator.
		itrOpt.Condition = influxql.RewriteExpr(influxql.CloneExpr(itrOpt.Condition), func(expr influxql.Expr) influxql.Expr {
			switch expr := expr.(type) {
			case *influxql.BinaryExpr:
				if ref, ok := expr.LHS.(*influxql.VarRef); ok && ref.Val == "_tagKey" {
					return &influxql.BooleanLiteral{Val: true}
				}
				if ref, ok := expr.RHS.(*influxql.VarRef); ok && ref.Val == "_tagKey" {
					return &influxql.BooleanLiteral{Val: true}
				}
			}
			return expr
		})
	}

	// Build conditional field cursors.
	// If a conditional field doesn't exist then ignore the series.
	var conds []cursorAt
	if len(conditionFields) > 0 {
		conds = make([]cursorAt, len(conditionFields))
		for i, ref := range conditionFields {
			// Create cursor from field if a tag wasn't requested.
			if ref.Type != influxql.Tag {
				cur := e.buildCursor(name, seriesKey, tfs, &ref, opt)
				if cur != nil {
					conds[i] = newBufCursor(cur, opt.Ascending)
					continue
				}

				// If a field was requested, use a nil cursor of the requested type.
				switch ref.Type {
				case influxql.Float, influxql.AnyField:
					conds[i] = nilFloatLiteralValueCursor
					continue
				case influxql.Integer:
					conds[i] = nilIntegerLiteralValueCursor
					continue
				case influxql.Unsigned:
					conds[i] = nilUnsignedLiteralValueCursor
					continue
				case influxql.String:
					conds[i] = nilStringLiteralValueCursor
					continue
				case influxql.Boolean:
					conds[i] = nilBooleanLiteralValueCursor
					continue
				}
			}

			// If field doesn't exist, use the tag value.
			if v := tags.Value(ref.Val); v == "" {
				// However, if the tag value is blank then return a null.
				conds[i] = nilStringLiteralValueCursor
			} else {
				conds[i] = &literalValueCursor{value: v}
			}
		}
	}
	condNames := influxql.VarRefs(conditionFields).Strings()

	// Limit tags to only the dimensions selected.
	dimensions := opt.GetDimensions()
	tags = tags.Subset(dimensions)

	// If it's only auxiliary fields then it doesn't matter what type of iterator we use.
	if ref == nil {
		if opt.StripName {
			name = ""
		}
		return newFloatIterator(name, tags, itrOpt, nil, aux, conds, condNames), nil
	}

	// Build main cursor.
	cur := e.buildCursor(name, seriesKey, tfs, ref, opt)

	// If the field doesn't exist then don't build an iterator.
	if cur == nil {
		cursorsAt(aux).close()
		cursorsAt(conds).close()
		return nil, nil
	}

	// Remove name if requested.
	if opt.StripName {
		name = ""
	}

	switch cur := cur.(type) {
	case floatCursor:
		return newFloatIterator(name, tags, itrOpt, cur, aux, conds, condNames), nil
	case integerCursor:
		return newIntegerIterator(name, tags, itrOpt, cur, aux, conds, condNames), nil
	case unsignedCursor:
		return newUnsignedIterator(name, tags, itrOpt, cur, aux, conds, condNames), nil
	case stringCursor:
		return newStringIterator(name, tags, itrOpt, cur, aux, conds, condNames), nil
	case booleanCursor:
		return newBooleanIterator(name, tags, itrOpt, cur, aux, conds, condNames), nil
	default:
		panic("unreachable")
	}
}

// buildCursor creates an untyped cursor for a field.
func (e *CacheEngine) buildCursor(measurement, seriesKey string, tags models.Tags, ref *influxql.VarRef, opt query.IteratorOptions) cursor {
	// Check if this is a system field cursor.
	switch ref.Val {
	case "_name":
		return &stringSliceCursor{values: []string{measurement}}
	case "_tagKey":
		return &stringSliceCursor{values: tags.Keys()}
	case "_tagValue":
		return &stringSliceCursor{values: matchTagValues(tags, opt.Condition)}
	case "_seriesKey":
		return &stringSliceCursor{values: []string{seriesKey}}
	}

	// Look up fields for measurement.
	mf := e.fieldset.Fields(measurement)
	if mf == nil {
		return nil
	}

	// Check for system field for field keys.
	if ref.Val == "_fieldKey" {
		return &stringSliceCursor{values: mf.FieldKeys()}
	}

	// Find individual field.
	f := mf.Field(ref.Val)
	if f == nil {
		return nil
	}

	// Check if we need to perform a cast. Performing a cast in the
	// engine (if it is possible) is much more efficient than an automatic cast.
	if ref.Type != influxql.Unknown && ref.Type != influxql.AnyField && ref.Type != f.Type {
		switch ref.Type {
		case influxql.Float:
			switch f.Type {
			case influxql.Integer:
				cur := e.buildIntegerCursor(measurement, seriesKey, ref.Val, opt)
				return &floatCastIntegerCursor{cursor: cur}
			case influxql.Unsigned:
				cur := e.buildUnsignedCursor(measurement, seriesKey, ref.Val, opt)
				return &floatCastUnsignedCursor{cursor: cur}
			}
		case influxql.Integer:
			switch f.Type {
			case influxql.Float:
				cur := e.buildFloatCursor(measurement, seriesKey, ref.Val, opt)
				return &integerCastFloatCursor{cursor: cur}
			case influxql.Unsigned:
				cur := e.buildUnsignedCursor(measurement, seriesKey, ref.Val, opt)
				return &integerCastUnsignedCursor{cursor: cur}
			}
		case influxql.Unsigned:
			switch f.Type {
			case influxql.Float:
				cur := e.buildFloatCursor(measurement, seriesKey, ref.Val, opt)
				return &unsignedCastFloatCursor{cursor: cur}
			case influxql.Integer:
				cur := e.buildIntegerCursor(measurement, seriesKey, ref.Val, opt)
				return &unsignedCastIntegerCursor{cursor: cur}
			}
		}
		return nil
	}

	// Return appropriate cursor based on type.
	switch f.Type {
	case influxql.Float:
		return e.buildFloatCursor(measurement, seriesKey, ref.Val, opt)
	case influxql.Integer:
		return e.buildIntegerCursor(measurement, seriesKey, ref.Val, opt)
	case influxql.Unsigned:
		return e.buildUnsignedCursor(measurement, seriesKey, ref.Val, opt)
	case influxql.String:
		return e.buildStringCursor(measurement, seriesKey, ref.Val, opt)
	case influxql.Boolean:
		return e.buildBooleanCursor(measurement, seriesKey, ref.Val, opt)
	default:
		panic("unreachable")
	}
}

// buildFloatCursor creates a cursor for a float field.
func (e *CacheEngine) buildFloatCursor(measurement, seriesKey, field string, opt query.IteratorOptions) floatCursor {
	key := SeriesFieldKeyBytes(seriesKey, field)
	cacheValues := e.Cache.Values(key)
	keyCursor := e.KeyCursor(key, opt.SeekTime(), opt.Ascending)
	return newFloatCursor(opt.SeekTime(), opt.Ascending, cacheValues, keyCursor)
}

// buildIntegerCursor creates a cursor for an integer field.
func (e *CacheEngine) buildIntegerCursor(measurement, seriesKey, field string, opt query.IteratorOptions) integerCursor {
	key := SeriesFieldKeyBytes(seriesKey, field)
	cacheValues := e.Cache.Values(key)
	keyCursor := e.KeyCursor(key, opt.SeekTime(), opt.Ascending)
	return newIntegerCursor(opt.SeekTime(), opt.Ascending, cacheValues, keyCursor)
}

// buildUnsignedCursor creates a cursor for an unsigned field.
func (e *CacheEngine) buildUnsignedCursor(measurement, seriesKey, field string, opt query.IteratorOptions) unsignedCursor {
	key := SeriesFieldKeyBytes(seriesKey, field)
	cacheValues := e.Cache.Values(key)
	keyCursor := e.KeyCursor(key, opt.SeekTime(), opt.Ascending)
	return newUnsignedCursor(opt.SeekTime(), opt.Ascending, cacheValues, keyCursor)
}

// buildStringCursor creates a cursor for a string field.
func (e *CacheEngine) buildStringCursor(measurement, seriesKey, field string, opt query.IteratorOptions) stringCursor {
	key := SeriesFieldKeyBytes(seriesKey, field)
	cacheValues := e.Cache.Values(key)
	keyCursor := e.KeyCursor(key, opt.SeekTime(), opt.Ascending)
	return newStringCursor(opt.SeekTime(), opt.Ascending, cacheValues, keyCursor)
}

// buildBooleanCursor creates a cursor for a boolean field.
func (e *CacheEngine) buildBooleanCursor(measurement, seriesKey, field string, opt query.IteratorOptions) booleanCursor {
	key := SeriesFieldKeyBytes(seriesKey, field)
	cacheValues := e.Cache.Values(key)
	keyCursor := e.KeyCursor(key, opt.SeekTime(), opt.Ascending)
	return newBooleanCursor(opt.SeekTime(), opt.Ascending, cacheValues, keyCursor)
}

// IteratorCost produces the cost of an iterator.
func (e *CacheEngine) IteratorCost(measurement string, opt query.IteratorOptions) (query.IteratorCost, error) {
	// Determine if this measurement exists. If it does not, then no shards are
	// accessed to begin with.
	if exists, err := e.index.MeasurementExists([]byte(measurement)); err != nil {
		return query.IteratorCost{}, err
	} else if !exists {
		return query.IteratorCost{}, nil
	}

	// Determine all of the tag sets for this query.
	tagSets, err := e.index.TagSets([]byte(measurement), opt)
	if err != nil {
		return query.IteratorCost{}, err
	}

	// Attempt to retrieve the ref from the main expression (if it exists).
	var ref *influxql.VarRef
	if opt.Expr != nil {
		if v, ok := opt.Expr.(*influxql.VarRef); ok {
			ref = v
		} else if call, ok := opt.Expr.(*influxql.Call); ok {
			if len(call.Args) > 0 {
				ref, _ = call.Args[0].(*influxql.VarRef)
			}
		}
	}

	// Count the number of series concatenated from the tag set.
	cost := query.IteratorCost{NumShards: 1}
	for _, t := range tagSets {
		cost.NumSeries += int64(len(t.SeriesKeys))
		for i, key := range t.SeriesKeys {
			// Retrieve the cost for the main expression (if it exists).
			if ref != nil {
				c := e.seriesCost(key, ref.Val, opt.StartTime, opt.EndTime)
				cost = cost.Combine(c)
			}

			// Retrieve the cost for every auxiliary field since these are also
			// iterators that we may have to look through.
			// We may want to separate these though as we are unlikely to incur
			// anywhere close to the full costs of the auxiliary iterators because
			// many of the selected values are usually skipped.
			for _, ref := range opt.Aux {
				c := e.seriesCost(key, ref.Val, opt.StartTime, opt.EndTime)
				cost = cost.Combine(c)
			}

			// Retrieve the expression names in the condition (if there is a condition).
			// We will also create cursors for these too.
			if t.Filters[i] != nil {
				refs := influxql.ExprNames(t.Filters[i])
				for _, ref := range refs {
					c := e.seriesCost(key, ref.Val, opt.StartTime, opt.EndTime)
					cost = cost.Combine(c)
				}
			}
		}
	}
	return cost, nil
}

func (e *CacheEngine) seriesCost(seriesKey, field string, tmin, tmax int64) query.IteratorCost {
	key := SeriesFieldKeyBytes(seriesKey, field)
	c := query.IteratorCost{}

	// Retrieve the range of values within the cache.
	cacheValues := e.Cache.Values(key)
	c.CachedValues = int64(len(cacheValues.Include(tmin, tmax)))
	return c
}

// SeriesPointIterator returns an iterator that iterates over available indices
func (e *CacheEngine) SeriesPointIterator(opt query.IteratorOptions) (query.Iterator, error) {
	return e.index.SeriesPointIterator(opt)
}

// CreateCursor is unused in the cache engine
func (e *CacheEngine) CreateCursor(ctx context.Context, r *tsdb.CursorRequest) (tsdb.Cursor, error) {
	return nil, nil
}

// SeriesAndFieldFromCompositeKey returns the series key and the field key extracted from the composite key.
func SeriesAndFieldFromCompositeKey(key []byte) ([]byte, []byte) {
	sep := bytes.Index(key, keyFieldSeparatorBytes)
	if sep == -1 {
		// No field???
		return key, nil
	}
	return key[:sep], key[sep+len(keyFieldSeparator):]
}
func varRefSliceContains(a []influxql.VarRef, v string) bool {
	for _, ref := range a {
		if ref.Val == v {
			return true
		}
	}
	return false
}

func varRefSliceRemove(a []influxql.VarRef, v string) []influxql.VarRef {
	if !varRefSliceContains(a, v) {
		return a
	}

	other := make([]influxql.VarRef, 0, len(a))
	for _, ref := range a {
		if ref.Val != v {
			other = append(other, ref)
		}
	}
	return other
}
func matchTagValues(tags models.Tags, condition influxql.Expr) []string {
	if condition == nil {
		return tags.Values()
	}

	// Populate map with tag values.
	data := map[string]interface{}{}
	for _, tag := range tags {
		data[string(tag.Key)] = string(tag.Value)
	}

	// Match against each specific tag.
	var values []string
	for _, tag := range tags {
		data["_tagKey"] = string(tag.Key)
		if influxql.EvalBool(condition, data) {
			values = append(values, string(tag.Value))
		}
	}
	return values
}

// SeriesFieldKeyBytes converts to byte array
func SeriesFieldKeyBytes(seriesKey, field string) []byte {
	b := make([]byte, len(seriesKey)+len(keyFieldSeparator)+len(field))
	i := copy(b[:], seriesKey)
	i += copy(b[i:], keyFieldSeparatorBytes)
	copy(b[i:], field)
	return b
}
