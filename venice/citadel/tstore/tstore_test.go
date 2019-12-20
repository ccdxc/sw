// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package tstore

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"testing"
	"time"

	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"
	meta2 "github.com/influxdata/influxdb/services/meta"
	"github.com/influxdata/influxdb/toml"
	"github.com/influxdata/influxdb/tsdb"
	_ "github.com/influxdata/influxdb/tsdb/engine"
	_ "github.com/influxdata/influxdb/tsdb/index"
	"github.com/influxdata/influxql"

	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var defaultDuration = time.Duration(meta.DefaultRetentionPeriod)

// ReadAllResults reads all results from c and returns as a slice.
func ReadAllResults(c <-chan *query.Result) []*query.Result {
	var a []*query.Result
	for result := range c {
		a = append(a, result)
	}
	return a
}

func TestTstoreBasic(t *testing.T) {
	// create a temp dir
	path, err := ioutil.TempDir("", "tstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create a new tstore
	ts, err := NewTstore(path)
	AssertOk(t, err, "Error creating tstore")
	defer ts.Close()

	// create the database
	err = ts.CreateDatabase("db1", &meta2.RetentionPolicySpec{
		Name:     meta.DefaultRetentionPolicyName,
		Duration: &defaultDuration,
	})
	AssertOk(t, err, "Error creatung the database")

	// read db
	dbs := ts.ReadDatabases()
	Assert(t, len(dbs) == 1, "invalid database", dbs)

	for _, db := range dbs {
		log.Infof("found db: %+v", db)
		Assert(t, db.Name == "db1", "invalid db", db)
	}

	// parse some points
	data := "cpu,host=serverB,svc=nginx value1=11,value2=12 \n" +
		"cpu,host=serverC,svc=nginx value1=21,value2=22  \n"
	points, err := models.ParsePointsWithPrecision([]byte(data), time.Now().UTC(), "ns")
	AssertOk(t, err, "Error parsing points")

	// write the points
	err = ts.WritePoints("db1", points)
	AssertOk(t, err, "Error writing points")

	// Retrieve shard group.
	shards := ts.tsdb.ShardGroup([]uint64{0, 1})

	// measurement
	measurement := &influxql.Measurement{
		Database:        "db1",
		RetentionPolicy: "default",
		Name:            "cpu",
	}

	// Create iterator.
	itr, err := shards.CreateIterator(context.Background(), measurement, query.IteratorOptions{
		Expr:       influxql.MustParseExpr(`value1`),
		Dimensions: []string{"host", "svc"},
		Ascending:  true,
		StartTime:  influxql.MinTime,
		EndTime:    influxql.MaxTime,
	})
	AssertOk(t, err, "Error creating iterator")
	defer itr.Close()

	// loop thru the iterator
	fitr := itr.(query.FloatIterator)
	p := &query.FloatPoint{}
	recCount := 0
	for err == nil && p != nil {
		p, err = fitr.Next()
		AssertOk(t, err, "Error getting next")
		log.Infof("Got p: %+v, err: %v", p, err)
		recCount++
	}
	Assert(t, recCount == 3, "got invalid number of records", recCount)

	// execute a query
	ch, err := ts.ExecuteQuery("SELECT * FROM cpu", "db1")
	AssertOk(t, err, "Error executing the query")

	rslt := ReadAllResults(ch)
	Assert(t, len(rslt) == 1, "got invalid number of results", rslt)
	Assert(t, len(rslt[0].Series) == 1, "got invalid number of series", rslt[0].Series)
	Assert(t, len(rslt[0].Series[0].Values) == 2, "got invalid number of values", rslt[0].Series[0].Values)

	// delete the database
	err = ts.DeleteDatabase("db1")
	AssertOk(t, err, "Error deleting database")
}

func TestTstoreBackupRetry(t *testing.T) {
	// create a temp dir
	path, err := ioutil.TempDir("", "tstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create a new tstore
	ts, err := NewTstore(path)
	AssertOk(t, err, "Error creating tstore")
	defer ts.Close()

	// create the database
	err = ts.CreateDatabase("db1", &meta2.RetentionPolicySpec{
		Name:     meta.DefaultRetentionPolicyName,
		Duration: &defaultDuration,
	})
	AssertOk(t, err, "Error creatung the database")

	// parse some points
	data := "cpu,host=serverB,svc=nginx value1=11,value2=12 \n" +
		"cpu,host=serverC,svc=nginx value1=21,value2=22  \n"
	points, err := models.ParsePointsWithPrecision([]byte(data), time.Now().UTC(), "n")
	AssertOk(t, err, "Error parsing points")

	// write the points
	err = ts.WritePoints("db1", points)
	AssertOk(t, err, "Error writing points")

	// execute a query and verify the results
	ch, err := ts.ExecuteQuery("SELECT * FROM cpu", "db1")
	AssertOk(t, err, "Error executing the query")
	rslt := ReadAllResults(ch)
	Assert(t, len(rslt) == 1, "got invalid number of results", rslt)
	Assert(t, len(rslt[0].Series) == 1, "got invalid number of series", rslt[0].Series)
	Assert(t, len(rslt[0].Series[0].Values) == 2, "got invalid number of values", rslt[0].Series[0].Values)

	// get shard info
	var sinfo tproto.SyncShardInfoMsg
	err = ts.GetShardInfo(&sinfo)
	AssertOk(t, err, "Error getting shard info")
	jstr, _ := json.MarshalIndent(sinfo, "", "  ")
	log.Infof("Got shard info: %s", jstr)

	// disable compaction
	for _, s := range ts.tsdb.Shards(ts.tsdb.ShardIDs()) {
		s.SetCompactionsEnabled(false)
	}

	for _, chunkInfo := range sinfo.Chunks {
		var buf bytes.Buffer

		// test backup ops
		err = ts.tsdb.BackupShard(chunkInfo.ChunkID, time.Unix(0, 0), &buf)
		Assert(t, err != nil, "backup didn't fail during compaction")

		// enable after a sec
		time.AfterFunc(time.Second, func() {
			for _, s := range ts.tsdb.Shards(ts.tsdb.ShardIDs()) {
				s.SetCompactionsEnabled(true)
			}
		})

		// backup chunk, blocks until compaction is enabled
		err = ts.BackupChunk(chunkInfo.ChunkID, &buf)
		AssertOk(t, err, "backup failed")
		break
	}
}

func TestTstoreWithConfig(t *testing.T) {
	// create a temp dir
	path, err := ioutil.TempDir("", "qstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	cfg := tsdb.NewConfig()

	// set custom parameters, test what is set in ts_data.go
	cfg.WALDir = filepath.Join(path, "wal")

	// set engine config
	cfg.CacheMaxMemorySize = 2 * tsdb.DefaultCacheMaxMemorySize
	cfg.CacheSnapshotMemorySize = 2 * tsdb.DefaultCacheMaxMemorySize
	cfg.CacheSnapshotWriteColdDuration = toml.Duration(time.Duration(time.Hour))
	cfg.CompactFullWriteColdDuration = toml.Duration(time.Duration(12 * time.Hour))

	// create a new qstore
	ts, err := NewTstoreWithConfig(path, cfg)
	AssertOk(t, err, "Error creating query store")
	defer ts.Close()

	// create the database
	err = ts.CreateDatabase("db1", &meta2.RetentionPolicySpec{
		Name:     meta.DefaultRetentionPolicyName,
		Duration: &defaultDuration,
	})
	AssertOk(t, err, "Error creatung the database")

	// parse some points
	data := "cpu,host=serverB,svc=nginx value1=11,value2=12 \n" +
		"cpu,host=serverC,svc=nginx value1=21,value2=22  \n"
	points, err := models.ParsePointsWithPrecision([]byte(data), time.Now().UTC(), "s")
	AssertOk(t, err, "Error parsing points")

	// write the points
	err = ts.WritePoints("db1", points)
	AssertOk(t, err, "Error writing points")

	// Retrieve shard group.
	shards := ts.tsdb.ShardGroup([]uint64{0, 1})

	// measurement
	measurement := &influxql.Measurement{
		Database:        "db1",
		RetentionPolicy: "default",
		Name:            "cpu",
	}

	// Create iterator.
	itr, err := shards.CreateIterator(context.Background(), measurement, query.IteratorOptions{
		Expr:       influxql.MustParseExpr(`value1`),
		Dimensions: []string{"host", "svc"},
		Ascending:  true,
		StartTime:  influxql.MinTime,
		EndTime:    influxql.MaxTime,
	})
	AssertOk(t, err, "Error creating iterator")
	defer itr.Close()

	// loop thru the iterator
	fitr := itr.(query.FloatIterator)
	p := &query.FloatPoint{}
	recCount := 0
	for err == nil && p != nil {
		p, err = fitr.Next()
		AssertOk(t, err, "Error getting next")
		log.Infof("Got p: %+v, err: %v", p, err)
		recCount++
	}
	Assert(t, recCount == 3, "got invalid number of records", recCount)

	// execute a query
	ch, err := ts.ExecuteQuery("SELECT * FROM cpu", "db1")
	AssertOk(t, err, "Error executing the query")

	rslt := ReadAllResults(ch)
	Assert(t, len(rslt) == 1, "got invalid number of results", rslt)
	Assert(t, len(rslt[0].Series) == 1, "got invalid number of series", rslt[0].Series)
	Assert(t, len(rslt[0].Series[0].Values) == 2, "got invalid number of values", rslt[0].Series[0].Values)

	// delete the database
	err = ts.DeleteDatabase("db1")
	AssertOk(t, err, "Error deleting database")
}

func TestTstoreBackupRestore(t *testing.T) {
	// create a temp dir
	path, err := ioutil.TempDir("", "tstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create a new tstore
	ts, err := NewTstore(path)
	AssertOk(t, err, "Error creating tstore")
	defer ts.Close()

	// create the database
	err = ts.CreateDatabase("db1", &meta2.RetentionPolicySpec{
		Name:     meta.DefaultRetentionPolicyName,
		Duration: &defaultDuration,
	})
	AssertOk(t, err, "Error creatung the database")

	// parse some points
	data := "cpu,host=serverB,svc=nginx value1=11,value2=12 \n" +
		"cpu,host=serverC,svc=nginx value1=21,value2=22  \n"
	points, err := models.ParsePointsWithPrecision([]byte(data), time.Now().UTC(), "n")
	AssertOk(t, err, "Error parsing points")

	// write the points
	err = ts.WritePoints("db1", points)
	AssertOk(t, err, "Error writing points")

	// execute a query and verify the results
	ch, err := ts.ExecuteQuery("SELECT * FROM cpu", "db1")
	AssertOk(t, err, "Error executing the query")

	rslt := ReadAllResults(ch)
	Assert(t, len(rslt) == 1, "got invalid number of results", rslt)
	Assert(t, len(rslt[0].Series) == 1, "got invalid number of series", rslt[0].Series)
	Assert(t, len(rslt[0].Series[0].Values) == 2, "got invalid number of values", rslt[0].Series[0].Values)

	// get shard info
	var sinfo tproto.SyncShardInfoMsg
	err = ts.GetShardInfo(&sinfo)
	AssertOk(t, err, "Error getting shard info")
	jstr, _ := json.MarshalIndent(sinfo, "", "  ")
	log.Infof("Got shard info: %s", jstr)

	// backup all the chunks
	backupBufs := make([]bytes.Buffer, len(sinfo.Chunks))
	for cid, chunkInfo := range sinfo.Chunks {
		var buf bytes.Buffer

		// backup chunk
		err = ts.BackupChunk(chunkInfo.ChunkID, &buf)
		AssertOk(t, err, "Error backing up chunk")
		backupBufs[cid] = buf
	}

	// create a new tstore
	path2, err := ioutil.TempDir("", "tstore2-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path2)

	// create a new tstore
	ts2, err := NewTstore(path2)
	AssertOk(t, err, "Error creating tstore")
	defer ts2.Close()

	// restore shard info new tstore
	err = ts2.RestoreShardInfo(&sinfo)
	AssertOk(t, err, "Error restoring shard info")

	// restore all the data
	for cid, chunkInfo := range sinfo.Chunks {
		err = ts2.RestoreChunk(chunkInfo.ChunkID, &backupBufs[cid])
		AssertOk(t, err, "Error restoring chunks")
	}

	// execute a query on restored DB and verify the results
	ch, err = ts2.ExecuteQuery("SELECT * FROM cpu", "db1")
	AssertOk(t, err, "Error executing the query")

	rslt = ReadAllResults(ch)
	Assert(t, len(rslt) == 1, "got invalid number of results", rslt)
	Assert(t, len(rslt[0].Series) == 1, "got invalid number of series", rslt[0].Series)
	Assert(t, len(rslt[0].Series[0].Values) == 2, "got invalid number of values", rslt[0].Series[0].Values)
}

func TestTstoreWithConfigBenchmark(t *testing.T) {
	var path string
	var err error

	// create a temp dir
	path, err = ioutil.TempDir("", "qstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	cfg := tsdb.NewConfig()

	// set custom parameters
	cfg.WALDir = filepath.Join(path, "wal")

	// set engine config

	// cache to 2 GB
	cfg.CacheMaxMemorySize = 2 * tsdb.DefaultCacheMaxMemorySize
	cfg.CacheSnapshotMemorySize = 2 * tsdb.DefaultCacheMaxMemorySize
	cfg.CacheSnapshotWriteColdDuration = toml.Duration(time.Duration(time.Hour))
	cfg.CompactFullWriteColdDuration = toml.Duration(time.Duration(12 * time.Hour))

	// create a new tstore
	ts, err := NewTstoreWithConfig(path, cfg)
	AssertOk(t, err, "Error creating tstore")
	defer ts.Close()
	tstoreBebchmark(t, ts)
}

func TestTstoreBenchmark(t *testing.T) {
	// create a temp dir
	path, err := ioutil.TempDir("", "tstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create a new tstore
	ts, err := NewTstore(path)
	AssertOk(t, err, "Error creating tstore")
	defer ts.Close()

	tstoreBebchmark(t, ts)
}

func tstoreBebchmark(t *testing.T, ts *Tstore) {
	var batchSize = 4000
	var numIterations = 100

	// create the database
	err := ts.CreateDatabase("db1", &meta2.RetentionPolicySpec{
		Name:     meta.DefaultRetentionPolicyName,
		Duration: &defaultDuration,
	})
	AssertOk(t, err, "Error creatung the database")

	// measure how long it takes to parse the points
	points := make([]models.Points, numIterations)
	startTime := time.Now()
	// pick a timestamp in the retention range, chose now() - 6 days
	pointStartTime := int(startTime.UnixNano() - time.Duration(6*24*time.Hour).Nanoseconds())

	for iter := 0; iter < numIterations; iter++ {
		// parse some points
		var data string
		for i := 0; i < batchSize; i++ {
			data += fmt.Sprintf("cpu%d,host=server%d,svc=nginx%d value1=%d,value2=%d %v\n", i, i, i, i+batchSize*iter, i+batchSize*iter, pointStartTime+(i+20+batchSize*iter))
		}
		points[iter], err = models.ParsePointsWithPrecision([]byte(data), time.Now().UTC(), "ns")
		AssertOk(t, err, "Error parsing points")
	}
	log.Infof("%d iterations at batch size %d parsing points took %v ", numIterations, batchSize, time.Since(startTime).String())

	// measure how long it takes to write the points
	startTime = time.Now()
	for iter := 0; iter < numIterations; iter++ {
		// write the points
		err = ts.WritePoints("db1", points[iter])
		AssertOk(t, err, "Error writing points")
	}
	log.Infof("%d iterations at batch size %d writing points took %v ", numIterations, batchSize, time.Since(startTime).String())

	// measure how long it takes to execute the query
	AssertEventually(t, func() (bool, interface{}) {
		for iter := 0; iter < batchSize; iter++ {
			// execute a query
			ch, err := ts.ExecuteQuery(fmt.Sprintf("SELECT * FROM cpu%d", iter), "db1")
			if err != nil {
				return false, err
			}

			rslt := ReadAllResults(ch)
			if len(rslt) != 1 {
				return false, fmt.Errorf("got invalid number of results")
			}
			if len(rslt[0].Series) != 1 {
				return false, fmt.Errorf("got invalid number of series")
			}
			if len(rslt[0].Series[0].Values) != numIterations {
				return false, fmt.Errorf("got invalid number of values")
			}
		}
		return true, nil
	}, "failed to query in replicas", "1s", "60s")

	startTime = time.Now()
	for iter := 0; iter < batchSize; iter++ {
		// execute a query
		ch, err := ts.ExecuteQuery(fmt.Sprintf("SELECT * FROM cpu%d", iter), "db1")
		AssertOk(t, err, "Error executing the query")

		rslt := ReadAllResults(ch)
		Assert(t, len(rslt) == 1, "got invalid number of results", rslt)
		Assert(t, len(rslt[0].Series) == 1, "got invalid number of series", rslt[0].Series)
		Assert(t, len(rslt[0].Series[0].Values) == numIterations, "got invalid number of values", len(rslt[0].Series[0].Values))
	}
	log.Infof("%d iterations at batch size %d executing query took %v ", numIterations, batchSize, time.Since(startTime).String())
}

func TestRetentionPolicy(t *testing.T) {
	// create a temp dir
	path, err := ioutil.TempDir("", "tstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create a new tstore
	ts, err := NewTstore(path)
	AssertOk(t, err, "Error creating tstore")
	defer ts.Close()

	// create the database
	err = ts.CreateDatabase("db1", nil)
	Assert(t, err != nil, "didn't fail for invalid retention policy")

	duration := time.Duration(100 * time.Hour)
	err = ts.CreateDatabase("db1", &meta2.RetentionPolicySpec{
		Name:     meta.DefaultRetentionPolicyName,
		Duration: &duration,
	})
	AssertOk(t, err, "Error creatung the database")
	dbs := ts.ReadDatabases()
	Assert(t, len(dbs) == 1, "invalid database", dbs)

	for _, db := range dbs {
		Assert(t, db.Name == "db1", "invalid db", db)
		Assert(t, db.RetentionPolicies[0].Duration == time.Duration(100*time.Hour), "invalid duration", db.RetentionPolicies)
	}
	ts.DeleteDatabase("db1")

	err = ts.CreateDatabase("db1", &meta2.RetentionPolicySpec{
		Name:     meta.DefaultRetentionPolicyName,
		Duration: &defaultDuration,
	})
	AssertOk(t, err, "Error creatung the database")
	defer ts.DeleteDatabase("db1")

	// read db
	dbs = ts.ReadDatabases()
	Assert(t, len(dbs) == 1, "invalid database", dbs)

	for _, db := range dbs {
		Assert(t, db.Name == "db1", "invalid db", db)
		Assert(t, db.RetentionPolicies[0].Duration == time.Duration(meta.DefaultRetentionPeriod), "invalid duration", db.RetentionPolicies)
	}
}
