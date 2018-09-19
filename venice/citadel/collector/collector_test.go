package collector

import (
	"context"
	"fmt"
	"testing"
	"time"

	"github.com/influxdata/influxdb/models"

	"github.com/pensando/sw/venice/citadel/collector/influxdb"
	"github.com/pensando/sw/venice/citadel/collector/mockdb"
	"github.com/pensando/sw/venice/citadel/collector/statssim"
	tu "github.com/pensando/sw/venice/utils/testutils"
)

const (
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
	dbServer, err := s.Setup()
	tu.AssertOk(t, err, "failed to setup mockdb")
	defer s.Teardown()

	cl, err := influxclient.NewInfluxClient(&influxclient.InfluxConfig{
		Addr: dbServer,
	})
	tu.AssertOk(t, err, fmt.Sprintf("failed to create influx client %s}", dbServer))

	c := NewCollector(cl)
	tu.AssertOk(t, err, fmt.Sprintf("failed to add backend {%s}", dbServer))

	ss := statssim.NewStatsSim()
	ss.Init()

	// write a single point
	p := getPoints(ss, 1)
	c.WritePoints(context.Background(), testDB, testMeas, p)

	tu.Assert(t, s.Writes == 1, "Expected 1 write got ", s.Writes)
	tu.Assert(t, s.PointsWritten == 1, "Expected 1 point got ", s.PointsWritten)

	// write a batch of points
	p = getPoints(ss, 10)
	c.WritePoints(context.Background(), testDB, testMeas, p)
	tu.Assert(t, s.Writes == 2, "Expected 2 write got ", s.Writes)
	tu.Assert(t, s.PointsWritten == 11, "Expected 11 point got ", s.PointsWritten)
	tu.Assert(t, len(s.LastLine) == 10, "Expected 10 points got ", len(s.LastLine))
}

func TestCollectorLine(t *testing.T) {
	s := &mockdb.MockTSDB{}
	dbServer, err := s.Setup()
	tu.AssertOk(t, err, "failed to setup mockdb")
	defer s.Teardown()

	cl, err := influxclient.NewInfluxClient(&influxclient.InfluxConfig{
		Addr: dbServer,
	})
	tu.AssertOk(t, err, fmt.Sprintf("failed to create influx client %s}", dbServer))
	c := NewCollector(cl)

	ss := statssim.NewStatsSim()
	ss.Init()

	// write a single point
	l := statssim.GetLinePoints(ss, testMeas, 1)
	c.WriteLines(context.Background(), testDB, l)

	tu.Assert(t, s.Writes == 1, "Expected 1 write got ", s.Writes)
	tu.Assert(t, s.PointsWritten == 1, "Expected 1 point got ", s.PointsWritten)

	// write a batch of points
	l = statssim.GetLinePoints(ss, testMeas, 10)
	c.WriteLines(context.Background(), testDB, l)
	tu.Assert(t, s.Writes == 2, "Expected 2 write got ", s.Writes)
	tu.Assert(t, s.PointsWritten == 11, "Expected 11 point got ", s.PointsWritten)
	tu.Assert(t, len(s.LastLine) == 10, "Expected 10 points got ", len(s.LastLine))
}
