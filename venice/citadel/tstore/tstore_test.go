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
	"strconv"
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
	AssertOk(t, err, "Error creating the database")

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
	AssertOk(t, err, "Error creating the database")

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
	AssertOk(t, err, "Error creating the database")

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
	AssertOk(t, err, "Error creating the database")

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
	AssertOk(t, err, "Error creating the database")

	// measure how long it takes to parse the points
	points := make([]models.Points, numIterations)
	startTime := time.Now()
	// pick a timestamp in the retention range, chose now() - 6 days
	pointStartTime := int(startTime.UnixNano() - time.Duration(0.5*24*time.Hour).Nanoseconds())

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
	AssertOk(t, err, "Error creating the database")
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
	AssertOk(t, err, "Error creating the database")
	defer ts.DeleteDatabase("db1")

	// read db
	dbs = ts.ReadDatabases()
	Assert(t, len(dbs) == 1, "invalid database", dbs)

	for _, db := range dbs {
		Assert(t, db.Name == "db1", "invalid db", db)
		Assert(t, db.RetentionPolicies[0].Duration == time.Duration(meta.DefaultRetentionPeriod), "invalid duration", db.RetentionPolicies)
	}
}

func TestTstoreContinuousQuery(t *testing.T) {
	// create a temp dir
	path, err := ioutil.TempDir("", "tstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create a new tstore
	ContinuousQueryRunInterval = time.Second
	ts, err := NewTstore(path)
	AssertOk(t, err, "Error creating tstore")
	defer ts.Close()

	// create the database
	err = ts.CreateDatabase("cqdb", &meta2.RetentionPolicySpec{
		Name:     meta.DefaultRetentionPolicyName,
		Duration: &defaultDuration,
	})
	AssertOk(t, err, "Error creating the database")

	// read db
	dbs := ts.ReadDatabases()
	Assert(t, len(dbs) == 1, "invalid database", dbs)

	for _, db := range dbs {
		log.Infof("found db: %+v", db)
		Assert(t, db.Name == "cqdb", "invalid db", db)
	}

	cqQuery := `CREATE CONTINUOUS QUERY "testcq" ON "cqdb" 
				BEGIN 
					SELECT mean("value") INTO "cqdb"."default"."average_value_five_seconds" 
					FROM "cpu" 
					GROUP BY time(5s) 
				END`
	err = ts.CreateContinuousQuery("cqdb", "testcq", "default", cqQuery)
	AssertOk(t, err, "Failed to create continuous query")

	log.Infof("Writing data for 1 min to test continunous query")
	for idx := 0; idx < 60; idx++ {
		data := "cpu,host=serverB,svc=nginx value=" + strconv.Itoa(idx) + "\n"
		points, err := models.ParsePointsWithPrecision([]byte(data), time.Now().UTC(), "ns")
		AssertOk(t, err, "Error parsing points")
		// write the points
		err = ts.WritePoints("cqdb", points)
		AssertOk(t, err, "Error writing points")
		time.Sleep(1 * time.Second)
	}

	ch, err := ts.ExecuteQuery("SELECT * FROM \"cqdb\".\"default\".\"average_value_five_seconds\"", "cqdb")
	results := ReadAllResults(ch)
	Assert(t, len(results) == 1, "Invalid number of result. Get %+v Expect %+v", len(results), 1)
	Assert(t, len(results[0].Series) > 0, "Invalid number of data points obtained")
	Assert(t, len(results[0].Series[0].Values) >= 11 && len(results[0].Series[0].Values) <= 13, "Invalid number of continuous query result. Get %+v Expect 11, 12 or 13", len(results[0].Series[0].Values))

	existed, err := ts.CheckContinuousQuery("cqdb", "testcq")
	AssertOk(t, err, "Error failed to check existence of continuous query in database")
	Assert(t, existed, "Error failed to get expected continuous query in database")

	cqs, err := ts.GetContinuousQuery("cqdb")
	AssertOk(t, err, "Error failed to get continuous query. Err: %v", err)
	Assert(t, len(cqs) == 1, "Invalid number of existed continuous query. Get %v Expect %v", len(cqs), 1)
	Assert(t, cqs[0].Name == "testcq", "Unexpected continuous query. Get %v Expect %v", cqs[0].Name, "testcq")
	Assert(t, cqs[0].Query == cqQuery, "Unexpected continuous query. Get %v Expect %v", cqs[0].Query, cqQuery)

	err = ts.DeleteContinuousQuery("cqdb", "testcq")
	AssertOk(t, err, "Error failed to delete continuous query. Err: %v", err)
	dbInfo := ts.metaClient.Database("cqdb")
	Assert(t, dbInfo != nil, "Unable to get database after deleting continuous query")
	Assert(t, len(dbInfo.ContinuousQueries) == 0, "Continuous query still exist after deleting operation")

	// Check previous continuous query result after deleting operation
	ch, err = ts.ExecuteQuery("SELECT * FROM \"cqdb\".\"default\".\"average_value_five_seconds\"", "cqdb")
	results = ReadAllResults(ch)
	Assert(t, len(results) == 1, "Invalid number of result. Get %+v Expect %+v", len(results), 1)
	Assert(t, len(results[0].Series[0].Values) >= 11 && len(results[0].Series[0].Values) <= 13, "Invalid number of continuous query result. Get %+v Expect 11, 12 or 13", len(results[0].Series[0].Values))

	// delete the database
	err = ts.DeleteDatabase("cqdb")
	AssertOk(t, err, "Error deleting database")
}

func TestTstoreContinuousQueryWithNewRetentionPolicy(t *testing.T) {
	// create a temp dir
	path, err := ioutil.TempDir("", "tstore-")
	AssertOk(t, err, "Error creating tmp dir")
	defer os.RemoveAll(path)

	// create a new tstore
	ts, err := NewTstore(path)
	AssertOk(t, err, "Error creating tstore")
	defer ts.Close()

	// create the database
	err = ts.CreateDatabase("cqdb", &meta2.RetentionPolicySpec{
		Name:     meta.DefaultRetentionPolicyName,
		Duration: &defaultDuration,
	})
	AssertOk(t, err, "Error creating the database")

	// read db
	dbs := ts.ReadDatabases()
	Assert(t, len(dbs) == 1, "invalid database", dbs)

	for _, db := range dbs {
		log.Infof("found db: %+v", db)
		Assert(t, db.Name == "cqdb", "invalid db", db)
	}

	cqQuery := `CREATE CONTINUOUS QUERY "testcq" ON "cqdb" 
				BEGIN 
					SELECT mean("value") INTO "cqdb"."new_rp"."average_value_five_seconds" 
					FROM "cpu" 
					GROUP BY time(5s) 
				END`
	err = ts.CreateContinuousQuery("cqdb", "testcq", "new_rp", cqQuery)
	Assert(t, err != nil, "Failed to raise error for creating continuous query on non-existed retention policy")

	// create new retention policy and make call on related api
	err = ts.CreateRetentionPolicy("cqdb", "new_rp", 7*24)
	AssertOk(t, err, "Failed to create retention policy for continuous query")
	err = ts.UpdateRetentionPolicy("cqdb", "new_rp", 1*24)
	AssertOk(t, err, "Failed to update retention policy for continuous query")
	rpInfoMap, err := ts.GetRetentionPolicy("cqdb")
	AssertOk(t, err, "Failed to get retention policy on database cqdb")
	Assert(t, len(rpInfoMap) == 2, "Invalid number of retention policy. Get %v, expect 2.", len(rpInfoMap))
	for rpName, rpDuration := range rpInfoMap {
		Assert(t, rpName == "default" || rpName == "new_rp", "Invalid name of obtained retention policy name. Get %v, expect default or new_rp", rpName)
		if rpName == "new_rp" {
			Assert(t, rpDuration == time.Duration(1*24)*time.Hour, "Incorrect retention duration obtained. Get %v Expect %v", rpDuration, time.Duration(1*24)*time.Hour)
		}
	}
	existed, err := ts.CheckRetentionPolicy("cqdb", "new_rp")
	AssertOk(t, err, "Failed to get retention policy in database")
	Assert(t, existed, "Failed to get new retention policy in database")

	// create continuous query after creating related retention policy
	err = ts.CreateContinuousQuery("cqdb", "testcq", "new_rp", cqQuery)
	AssertOk(t, err, "Failed to create continuous query after creating related retention policy")

	log.Infof("Writing data for 1 min to test continunous query")
	for idx := 0; idx < 60; idx++ {
		data := "cpu,host=serverB,svc=nginx value=" + strconv.Itoa(idx) + "\n"
		points, err := models.ParsePointsWithPrecision([]byte(data), time.Now().UTC(), "ns")
		AssertOk(t, err, "Error parsing points")
		// write the points
		err = ts.WritePoints("cqdb", points)
		AssertOk(t, err, "Error writing points")
		time.Sleep(1 * time.Second)
	}

	ch, err := ts.ExecuteQuery("SELECT * FROM \"cqdb\".\"new_rp\".\"average_value_five_seconds\"", "cqdb")
	results := ReadAllResults(ch)
	Assert(t, len(results) == 1, "Invalid number of result. Get %+v Expect %+v", len(results), 1)
	Assert(t, len(results[0].Series[0].Values) >= 11 && len(results[0].Series[0].Values) <= 13, "Invalid number of continuous query result. Get %+v Expect 11, 12 or 13", len(results[0].Series[0].Values))

	cqs, err := ts.GetContinuousQuery("cqdb")
	AssertOk(t, err, "Error failed to get continuous query. Err: %v", err)
	Assert(t, len(cqs) == 1, "Invalid number of existed continuous query. Get %v Expect %v", len(cqs), 1)
	Assert(t, cqs[0].Name == "testcq", "Unexpected continuous query. Get %v Expect %v", cqs[0].Name, "testcq")
	Assert(t, cqs[0].Query == cqQuery, "Unexpected continuous query. Get %v Expect %v", cqs[0].Query, cqQuery)

	err = ts.DeleteContinuousQuery("cqdb", "testcq")
	AssertOk(t, err, "Error failed to delete continuous query. Err: %v", err)
	err = ts.DeleteRetentionPolicy("cqdb", "new_rp")
	AssertOk(t, err, "Error failed to delete retention policy. Err: %v", err)
	dbInfo := ts.metaClient.Database("cqdb")
	Assert(t, dbInfo != nil, "Unable to get database after deleting continuous query")
	Assert(t, len(dbInfo.ContinuousQueries) == 0, "Continuous query still exist after deleting operation")
	Assert(t, len(dbInfo.RetentionPolicies) == 1, "Only default retention policy should remain in database")

	// delete the database
	err = ts.DeleteDatabase("cqdb")
	AssertOk(t, err, "Error deleting database")

	// Mock error for retention policy related api
	err = ts.CreateRetentionPolicy("unknowndb", "new_rp", 7*24)
	Assert(t, err != nil, "Failed to raise error for creating retention policy on non-existed database")
	rpInfoMap, err = ts.GetRetentionPolicy("unknowndb")
	Assert(t, len(rpInfoMap) == 0 && err != nil, "Failed to raise error for trying to get retention policy on non-existed database")
	existed, err = ts.CheckRetentionPolicy("uknowndb", "new_rp")
	Assert(t, err != nil, "Failed to raise error for checking retention policy on non-existed database")
	err = ts.DeleteRetentionPolicy("unknowndb", "new_rp")
	Assert(t, err != nil, "Failed to raise error for deleting retention policy on non-existed database")
}
