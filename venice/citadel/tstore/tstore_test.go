// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package tstore

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"testing"
	"time"

	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"
	_ "github.com/influxdata/influxdb/tsdb/engine"
	_ "github.com/influxdata/influxdb/tsdb/index"
	"github.com/influxdata/influxql"

	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

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
	err = ts.CreateDatabase("db1", nil)
	AssertOk(t, err, "Error creatung the database")

	// parse some points
	data := "cpu,host=serverB,svc=nginx value1=11,value2=12 10\n" +
		"cpu,host=serverC,svc=nginx value1=21,value2=22  20\n"
	points, err := models.ParsePointsWithPrecision([]byte(data), time.Time{}, "s")
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
	err = ts.CreateDatabase("db1", nil)
	AssertOk(t, err, "Error creatung the database")

	// parse some points
	data := "cpu,host=serverB,svc=nginx value1=11,value2=12 10\n" +
		"cpu,host=serverC,svc=nginx value1=21,value2=22  20\n"
	points, err := models.ParsePointsWithPrecision([]byte(data), time.Time{}, "n")
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

func TestTstoreBenchmark(t *testing.T) {
	var batchSize = 4000
	var numIterations = 100

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
	AssertOk(t, err, "Error creatung the database")

	// measure how long it takes to parse the points
	points := make([]models.Points, numIterations)
	startTime := time.Now()
	for iter := 0; iter < numIterations; iter++ {
		// parse some points
		var data string
		for i := 0; i < batchSize; i++ {
			data += fmt.Sprintf("cpu%d,host=server%d,svc=nginx%d value1=%d,value2=%d %d\n", i, i, i, (i + batchSize*iter), (i + batchSize*iter), (i + 20 + batchSize*iter))
		}
		points[iter], err = models.ParsePointsWithPrecision([]byte(data), time.Now(), "s")
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
