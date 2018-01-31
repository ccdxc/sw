package vstore

// vstore implements a volatile TSDB store to be accessed
// by PointsWriter and QueryExecutor

import (
	"bytes"
	"errors"
	"fmt"
	"io"
	"sort"
	"strings"
	"sync"
	"time"

	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"
	"github.com/influxdata/influxdb/tsdb"
	"github.com/influxdata/influxql"

	"github.com/pensando/sw/venice/aggregator/cachedb"
)

// Store information about the volatile store
type Store struct {
	mu sync.RWMutex
	// databases keeps track of the number of databases being managed by the store.
	databases map[string]struct{}
	// shared per-database indexes, because "inmem" index is used.
	indexes map[string]interface{}

	// shards is a map of shard IDs to the associated Shard.
	shards        map[uint64]*Shard
	EngineOptions tsdb.EngineOptions
}

// NewStore returns a new store with default configuration.
func NewStore() *Store {
	return &Store{
		databases:     make(map[string]struct{}),
		indexes:       make(map[string]interface{}),
		shards:        make(map[uint64]*Shard),
		EngineOptions: tsdb.NewEngineOptions(),
	}
}

// CreateShard creates a shard with the given id and retention policy on a database.
func (s *Store) CreateShard(database, retentionPolicy string, shardID uint64, enabled bool) error {
	s.mu.Lock()
	defer s.mu.Unlock()

	// Shard already exists.
	if _, ok := s.shards[shardID]; ok {
		return nil
	}

	// Retrieve shared index, if needed.
	idx, err := s.createIndexIfNotExists(database)
	if err != nil {
		return err
	}

	// Copy index options and pass in shared index.
	opt := s.EngineOptions
	opt.InmemIndex = idx

	shard := NewShard(shardID, database, retentionPolicy, opt)
	shard.EnableOnOpen = enabled

	if err := shard.Open(); err != nil {
		return err
	}

	s.shards[shardID] = shard
	s.databases[database] = struct{}{} // Ensure we are tracking any new db.

	return nil
}

// CreateShardSnapshot is n/a
func (s *Store) CreateShardSnapshot(id uint64) (string, error) {
	return "", nil
}

// WriteToShard writes a list of points to a shard identified by its ID.
func (s *Store) WriteToShard(shardID uint64, points []models.Point) error {
	s.mu.RLock()

	sh := s.shards[shardID]
	if sh == nil {
		s.mu.RUnlock()
		return tsdb.ErrShardNotFound
	}
	s.mu.RUnlock()

	return sh.WritePoints(points)
}

// RestoreShard is n/a
func (s *Store) RestoreShard(id uint64, r io.Reader) error {
	return nil
}

//BackupShard is n/a
func (s *Store) BackupShard(id uint64, since time.Time, w io.Writer) error {
	return nil
}

// DeleteDatabase is n/a for now
func (s *Store) DeleteDatabase(name string) error {
	return nil
}

// DeleteMeasurement is n/a for now
func (s *Store) DeleteMeasurement(database, name string) error {
	return nil
}

// DeleteRetentionPolicy is n/a
func (s *Store) DeleteRetentionPolicy(database, name string) error {
	return nil
}

// DeleteSeries is n/a
func (s *Store) DeleteSeries(database string, sources []influxql.Source, condition influxql.Expr) error {
	return nil
}

// DeleteShard is n/a
func (s *Store) DeleteShard(shardID uint64) error {
	return nil
}

// MeasurementNames is n/a
func (s *Store) MeasurementNames(database string, cond influxql.Expr) ([][]byte, error) {
	return nil, nil
}

// tagValues is a temporary representation of a TagValues. Rather than allocating
// KeyValues as we build up a TagValues object, We hold off allocating KeyValues
// until we have merged multiple tagValues together.
type tagValues struct {
	name   []byte
	keys   []string
	values [][]string
}

// Is a slice of tagValues that can be sorted by measurement.
type tagValuesSlice []tagValues

func (a tagValuesSlice) Len() int           { return len(a) }
func (a tagValuesSlice) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }
func (a tagValuesSlice) Less(i, j int) bool { return bytes.Compare(a[i].name, a[j].name) == -1 }

// TagKeys is not used
func (s *Store) TagKeys(auth query.Authorizer, shardIDs []uint64, cond influxql.Expr) ([]tsdb.TagKeys, error) {
	return nil, nil
}

// TagValues returns the tag keys and values in the given database, matching the condition.
func (s *Store) TagValues(auth query.Authorizer, shardIDs []uint64, cond influxql.Expr) ([]tsdb.TagValues, error) {
	if cond == nil {
		return nil, errors.New("a condition is required")
	}

	measurementExpr := influxql.CloneExpr(cond)
	measurementExpr = influxql.Reduce(influxql.RewriteExpr(measurementExpr, func(e influxql.Expr) influxql.Expr {
		switch e := e.(type) {
		case *influxql.BinaryExpr:
			switch e.Op {
			case influxql.EQ, influxql.NEQ, influxql.EQREGEX, influxql.NEQREGEX:
				tag, ok := e.LHS.(*influxql.VarRef)
				if !ok || tag.Val != "_name" {
					return nil
				}
			}
		}
		return e
	}), nil)

	filterExpr := influxql.CloneExpr(cond)
	filterExpr = influxql.Reduce(influxql.RewriteExpr(filterExpr, func(e influxql.Expr) influxql.Expr {
		switch e := e.(type) {
		case *influxql.BinaryExpr:
			switch e.Op {
			case influxql.EQ, influxql.NEQ, influxql.EQREGEX, influxql.NEQREGEX:
				tag, ok := e.LHS.(*influxql.VarRef)
				if !ok || strings.HasPrefix(tag.Val, "_") {
					return nil
				}
			}
		}
		return e
	}), nil)

	// If we're using the inmem index then all shards contain a duplicate
	// version of the global index. We don't need to iterate over all shards
	shards := s.Shards(shardIDs)
	// since we have everything we need from the first shard.
	if s.EngineOptions.IndexVersion == "inmem" && len(shards) > 0 {
		shards = shards[:1]
	}

	// Stores each list of TagValues for each measurement.
	var allResults []tagValues
	var maxMeasurements int // Hint as to lower bound on number of measurements.
	for _, sh := range shards {
		// names will be sorted by MeasurementNamesByExpr.
		names, err := sh.MeasurementNamesByExpr(measurementExpr)
		if err != nil {
			return nil, err
		}

		if len(names) > maxMeasurements {
			maxMeasurements = len(names)
		}

		if allResults == nil {
			allResults = make([]tagValues, 0, len(shards)*len(names)) // Assuming all series in all shards.
		}

		// Iterate over each matching measurement in the shard. For each
		// measurement we'll get the matching tag keys (e.g., when a WITH KEYS)
		// statement is used, and we'll then use those to fetch all the relevant
		// values from matching series. Series may be filtered using a WHERE
		// filter.
		for _, name := range names {
			// Determine a list of keys from condition.
			keySet, err := sh.MeasurementTagKeysByExpr(name, cond)
			if err != nil {
				return nil, err
			}

			if len(keySet) == 0 {
				// No matching tag keys for this measurement
				continue
			}

			result := tagValues{
				name: name,
				keys: make([]string, 0, len(keySet)),
			}

			// Add the keys to the tagValues and sort them.
			for k := range keySet {
				result.keys = append(result.keys, k)
			}
			sort.Sort(sort.StringSlice(result.keys))

			// get all the tag values for each key in the keyset.
			// Each slice in the results contains the sorted values associated
			// associated with each tag key for the measurement from the key set.
			if result.values, err = sh.MeasurementTagKeyValuesByExpr(auth, name, result.keys, filterExpr, true); err != nil {
				return nil, err
			}

			// remove any tag keys that didn't have any authorized values
			j := 0
			for i := range result.keys {
				if len(result.values[i]) == 0 {
					continue
				}

				result.keys[j] = result.keys[i]
				result.values[j] = result.values[i]
				j++
			}
			result.keys = result.keys[:j]
			result.values = result.values[:j]

			// only include result if there are keys with values
			if len(result.keys) > 0 {
				allResults = append(allResults, result)
			}
		}
	}

	result := make([]tsdb.TagValues, 0, maxMeasurements)

	// We need to sort all results by measurement name.
	if len(shards) > 1 {
		sort.Sort(tagValuesSlice(allResults))
	}

	// The next stage is to merge the tagValue results for each shard's measurements.
	var i, j int
	// Used as a temporary buffer in mergeTagValues. There can be at most len(shards)
	// instances of tagValues for a given measurement.
	idxBuf := make([][2]int, 0, len(shards))
	for i < len(allResults) {
		// Gather all occurrences of the same measurement for merging.
		for j+1 < len(allResults) && bytes.Equal(allResults[j+1].name, allResults[i].name) {
			j++
		}

		// An invariant is that there can't be more than n instances of tag
		// key value pairs for a given measurement, where n is the number of
		// shards.
		if got, exp := j-i+1, len(shards); got > exp {
			return nil, fmt.Errorf("unexpected results returned engine. Got %d measurement sets for %d shards", got, exp)
		}

		nextResult := mergeTagValues(idxBuf, allResults[i:j+1]...)
		i = j + 1
		if len(nextResult.Values) > 0 {
			result = append(result, nextResult)
		}
	}
	return result, nil
}

// mergeTagValues merges multiple sorted sets of temporary tagValues using a
// direct k-way merge whilst also removing duplicated entries. The result is a
// single TagValue type.
//
// TODO(edd): a Tournament based merge (see: Knuth's TAOCP 5.4.1) might be more
// appropriate at some point.
//
func mergeTagValues(valueIdxs [][2]int, tvs ...tagValues) tsdb.TagValues {
	var result tsdb.TagValues
	if len(tvs) == 0 {
		return tsdb.TagValues{}
	} else if len(tvs) == 1 {
		result.Measurement = string(tvs[0].name)
		// TODO(edd): will be too small likely. Find a hint?
		result.Values = make([]tsdb.KeyValue, 0, len(tvs[0].values))

		for ki, key := range tvs[0].keys {
			for _, value := range tvs[0].values[ki] {
				result.Values = append(result.Values, tsdb.KeyValue{Key: key, Value: value})
			}
		}
		return result
	}

	result.Measurement = string(tvs[0].name)

	var maxSize int
	for _, tv := range tvs {
		if len(tv.values) > maxSize {
			maxSize = len(tv.values)
		}
	}
	result.Values = make([]tsdb.KeyValue, 0, maxSize) // This will likely be too small but it's a start.

	// Resize and reset to the number of TagValues we're merging.
	valueIdxs = valueIdxs[:len(tvs)]
	for i := 0; i < len(valueIdxs); i++ {
		valueIdxs[i][0], valueIdxs[i][1] = 0, 0
	}

	var (
		j              int
		keyCmp, valCmp int
	)

	for {
		// Which of the provided TagValue sets currently holds the smallest element.
		// j is the candidate we're going to next pick for the result set.
		j = -1

		// Find the smallest element
		for i := 0; i < len(tvs); i++ {
			if valueIdxs[i][0] >= len(tvs[i].keys) {
				continue // We have completely drained all tag keys and values for this shard.
			} else if len(tvs[i].values[valueIdxs[i][0]]) == 0 {
				// There are no tag values for these keys.
				valueIdxs[i][0]++
				valueIdxs[i][1] = 0
				continue
			} else if j == -1 {
				// We haven't picked a best TagValues set yet. Pick this one.
				j = i
				continue
			}

			// It this tag key is lower than the candidate's tag key
			keyCmp = strings.Compare(tvs[i].keys[valueIdxs[i][0]], tvs[j].keys[valueIdxs[j][0]])
			if keyCmp == -1 {
				j = i
			} else if keyCmp == 0 {
				valCmp = strings.Compare(tvs[i].values[valueIdxs[i][0]][valueIdxs[i][1]], tvs[j].values[valueIdxs[j][0]][valueIdxs[j][1]])
				// Same tag key but this tag value is lower than the candidate.
				if valCmp == -1 {
					j = i
				} else if valCmp == 0 {
					// Duplicate tag key/value pair.... Remove and move onto
					// the next value for shard i.
					valueIdxs[i][1]++
					if valueIdxs[i][1] >= len(tvs[i].values[valueIdxs[i][0]]) {
						// Drained all these tag values, move onto next key.
						valueIdxs[i][0]++
						valueIdxs[i][1] = 0
					}
				}
			}
		}

		// We could have drained all of the TagValue sets and be done...
		if j == -1 {
			break
		}

		// Append the smallest KeyValue
		result.Values = append(result.Values, tsdb.KeyValue{
			Key:   string(tvs[j].keys[valueIdxs[j][0]]),
			Value: tvs[j].values[valueIdxs[j][0]][valueIdxs[j][1]],
		})
		// Increment the indexes for the chosen TagValue.
		valueIdxs[j][1]++
		if valueIdxs[j][1] >= len(tvs[j].values[valueIdxs[j][0]]) {
			// Drained all these tag values, move onto next key.
			valueIdxs[j][0]++
			valueIdxs[j][1] = 0
		}
	}
	return result
}

// createIndexIfNotExists returns a shared index for a database, if the inmem
// index is being used. If the TSI index is being used, then this method is
// basically a no-op.
func (s *Store) createIndexIfNotExists(name string) (interface{}, error) {
	if idx := s.indexes[name]; idx != nil {
		return idx, nil
	}

	idx, err := tsdb.NewInmemIndex(name)
	if err != nil {
		return nil, err
	}

	s.indexes[name] = idx
	return idx, nil
}

// filterShards returns a slice of shards where fn returns true
// for the shard. If the provided predicate is nil then all shards are returned.
func (s *Store) filterShards(fn func(sh *Shard) bool) []*Shard {
	var shards []*Shard
	if fn == nil {
		shards = make([]*Shard, 0, len(s.shards))
		fn = func(*Shard) bool { return true }
	} else {
		shards = make([]*Shard, 0)
	}

	for _, sh := range s.shards {
		if fn(sh) {
			shards = append(shards, sh)
		}
	}
	return shards
}

// byDatabase provides a predicate for filterShards that matches on the name of
// the database passed in.
func byDatabase(name string) func(sh *Shard) bool {
	return func(sh *Shard) bool {
		return sh.database == name
	}
}

// MeasurementsCardinality is unused
func (s *Store) MeasurementsCardinality(database string) (int64, error) {
	return 0, errors.New("shard nil, can't get cardinality")
}

// SeriesCardinality is unused
func (s *Store) SeriesCardinality(database string) (int64, error) {
	return 0, errors.New("shard nil, can't get cardinality")
}

// ShardGroup returns a ShardGroup with a list of shards by id.
func (s *Store) ShardGroup(ids []uint64) tsdb.ShardGroup {
	return Shards(s.Shards(ids))
}

// Shards returns a list of shards by id.
func (s *Store) Shards(ids []uint64) []*Shard {
	s.mu.RLock()
	defer s.mu.RUnlock()
	a := make([]*Shard, 0, len(ids))
	for _, id := range ids {
		sh, ok := s.shards[id]
		if !ok {
			continue
		}
		a = append(a, sh)
	}
	return a
}

// NewEngine creates a cache engine
func NewEngine(id uint64, i tsdb.Index, database, path string, walPath string, options tsdb.EngineOptions) (tsdb.Engine, error) {
	return cachedb.NewCacheEngine(id, i, database, path, walPath, options), nil
}
