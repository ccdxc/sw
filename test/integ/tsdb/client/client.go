package client

import (
	"context"
	"fmt"
	"time"

	"github.com/pensando/sw/test/integ/tsdb/collector"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ntsdb"
)

const (
	testPeriod                  = 50 * time.Millisecond
	testSendInterval            = 10 * time.Millisecond
	testConnectionRetryInterval = 10 * time.Millisecond
	testTenant                  = "test-tenant"
)

// Suite keeps per test suite context for teardown, etc.
type Suite struct {
	cs         *collectorinteg.Suite
	context    context.Context
	cancelFunc context.CancelFunc
}

// NewSuite creates a new test suite with client, collector and db
func NewSuite(numBE int, url string, batchPeriod time.Duration, testName, dbName string) *Suite {
	ts := &Suite{}
	cs := collectorinteg.NewSuite(numBE, url, batchPeriod)
	cs.CreateDB(dbName)

	// setup client
	_, ts.cancelFunc = context.WithCancel(context.Background())
	ntsdb.Init(context.Background(), &ntsdb.Opts{ClientName: testName,
		Collector:               cs.CollectorURL(),
		SendInterval:            testSendInterval,
		ConnectionRetryInterval: testConnectionRetryInterval,
		DBName:                  dbName,
	})
	time.Sleep(3 * testConnectionRetryInterval)

	ts.cs = cs
	return ts
}

// TearDown cleans up tsdb, client threads and collector process
func (ts *Suite) TearDown() {
	ts.cs.TearDown()
	ts.cancelFunc()
	ntsdb.Cleanup()
}

func validate(ts *Suite, dbName, measName string, tt *collectorinteg.TimeTable) (bool, error) {
	res, err := ts.cs.Query(0, dbName, fmt.Sprintf("SELECT * FROM %s", measName))
	if err != nil {
		log.Errorf("error querying tsdb: %s", err)
		return false, err
	}
	if len(res[0].Series) == 0 {
		log.Infof("res: %+v, tt: %+v", res, tt)
		return false, nil
	}
	err = tt.MatchQueryRow(res[0].Series[0])
	if err != nil {
		log.Infof("res: %+v, tt: %+v", res, tt)
		return false, err
	}
	return true, nil
}

func getCount(ts *Suite, dbName, measName string) (int, error) {
	res, err := ts.cs.Query(0, dbName, fmt.Sprintf("SELECT * FROM %s", measName))
	if err != nil {
		log.Errorf("error querying tsdb: %s", err)
		return 0, err
	}

	totalCount := 0
	for _, resi := range res {
		for _, seriesi := range resi.Series {
			totalCount += len(seriesi.Values)
		}
	}
	return totalCount, nil
}
