package collector

import (
	"context"
	"reflect"
	"sync"
	"testing"
	"time"

	"github.com/influxdata/influxdb/models"

	"github.com/pensando/sw/venice/collector/mockdb"

	"github.com/pensando/sw/venice/collector/statssim"
	tu "github.com/pensando/sw/venice/utils/testutils"
)

const (
	portA    = "localhost:8086"
	portB    = "localhost:18086"
	testDB   = "t-e-s-t-D-B"
	testMeas = "testStats"
)

func getPoints(ss *statssim.StatsSim, count int) []models.Point {
	res := make([]models.Point, 0, count)

	for ix := 0; ix < count; ix++ {
		tags, fields := ss.GetFlowStats()
		mTags := models.NewTags(tags)
		p, _ := models.NewPoint(testMeas, mTags, fields, time.Now())
		res = append(res, p)
	}

	return res
}

func TestCollectorBasic(t *testing.T) {
	s := &mockdb.MockTSDB{}
	s.Setup(portA)
	defer s.Teardown()

	c := NewCollector(context.Background()).WithPeriod(100 * time.Millisecond)
	err := c.AddBackEnd("http://" + portA)
	if err != nil {
		t.Fatalf(err.Error())
	}

	ss := statssim.NewStatsSim()
	ss.Init()

	// write a single point
	p := getPoints(ss, 1)
	c.WritePoints(testDB, testMeas, p)

	tu.Assert(t, s.Writes == 1, "Expected 1 write got ", s.Writes)
	tu.Assert(t, s.PointsWritten == 1, "Expected 1 point got ", s.PointsWritten)

	// write a batch of points
	p = getPoints(ss, 10)
	c.WritePoints(testDB, testMeas, p)
	tu.Assert(t, s.Writes == 2, "Expected 2 write got ", s.Writes)
	tu.Assert(t, s.PointsWritten == 11, "Expected 11 point got ", s.PointsWritten)
	tu.Assert(t, len(s.LastLine) == 10, "Expected 10 points got ", len(s.LastLine))
}

func TestCollectorDual(t *testing.T) {
	// start two servers
	s1 := &mockdb.MockTSDB{}
	s1.Setup(portA)

	s2 := &mockdb.MockTSDB{}
	s2.Setup(portB)
	defer s2.Teardown()

	c := NewCollector(context.Background()).WithPeriod(100 * time.Millisecond).WithSize(12)

	// Add both backends
	err := c.AddBackEnd("http://" + portA)
	if err != nil {
		t.Fatalf(err.Error())
	}

	err = c.AddBackEnd("http://" + portB)
	if err != nil {
		t.Fatalf(err.Error())
	}

	ss := statssim.NewStatsSim()
	ss.Init()

	// write a single point
	p := getPoints(ss, 1)
	c.WritePoints(testDB, testMeas, p)

	tu.Assert(t, s1.Writes == 1, "Expected 1 write got ", s1.Writes)
	tu.Assert(t, s2.Writes == 1, "Expected 1 write got ", s2.Writes)
	tu.Assert(t, reflect.DeepEqual(s1.LastLine, s2.LastLine), "LastLines did not match")

	// write 3 sets
	var wg sync.WaitGroup
	for ix := 0; ix < 3; ix++ {
		p = getPoints(ss, 6)
		wg.Add(1)
		go func(p []models.Point) {
			defer wg.Done()
			c.WritePoints(testDB, testMeas, p)
		}(p)
	}

	wg.Wait()
	tu.Assert(t, s1.Writes == 3, "Expected 3 writes got ", s1.Writes)
	tu.Assert(t, s1.PointsWritten == 19, "Expected 19 points got ", s1.PointsWritten)
	tu.Assert(t, len(s1.LastLine) == 6, "Expected 6 points got ", len(s1.LastLine))
	tu.Assert(t, s2.Writes == 3, "Expected 3 writes got ", s2.Writes)
	tu.Assert(t, s2.PointsWritten == 19, "Expected 19 points got ", s2.PointsWritten)
	tu.Assert(t, len(s2.LastLine) == 6, "Expected 6 points got ", len(s2.LastLine))
	tu.Assert(t, reflect.DeepEqual(s1.LastLine, s2.LastLine), "LastLines did not match")
	// kill one backend
	s1.Teardown()

	// write 3 sets
	for ix := 0; ix < 3; ix++ {
		p = getPoints(ss, 6)
		wg.Add(1)
		go func(p []models.Point) {
			defer wg.Done()
			c.WritePoints(testDB, testMeas, p)
		}(p)
	}
	wg.Wait()
	tu.Assert(t, s2.Writes == 5, "Expected 5 writes got ", s2.Writes)
	tu.Assert(t, s2.PointsWritten == 37, "Expected 37 points got ", s2.PointsWritten)
	tu.Assert(t, len(s2.LastLine) == 6, "Expected 6 points got ", len(s2.LastLine))
	tu.Assert(t, s1.Writes == 3, "Expected 3 writes got ", s1.Writes)
}
