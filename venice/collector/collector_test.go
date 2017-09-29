package collector

import (
	"bytes"
	"context"
	"fmt"
	"github.com/gorilla/mux"
	"github.com/influxdata/influxdb/models"
	"net/http"
	"reflect"
	"sync"
	"testing"
	"time"

	"github.com/pensando/sw/venice/collector/statssim"
	"github.com/pensando/sw/venice/utils/log"
	n "github.com/pensando/sw/venice/utils/netutils"
	tu "github.com/pensando/sw/venice/utils/testutils"
)

const (
	portA    = "localhost:8086"
	portB    = "localhost:18086"
	testDB   = "t-e-s-t-D-B"
	testMeas = "testStats"
)

type testInfServer struct {
	url           string
	srv           *http.Server
	doneCh        chan bool
	pointsWritten uint64
	writes        uint64
	lastLine      []models.Point
}

func (i *testInfServer) Setup(url string) {

	testWriter := func(r *http.Request) (interface{}, error) {
		body := r.Body
		if r.Header.Get("Content-Encoding") == "gzip" {
			log.Infof("gzip encoding detected")
		}

		if r.ContentLength <= 0 {
			return nil, fmt.Errorf("Bad content length %d", r.ContentLength)
		}

		bs := make([]byte, 0, r.ContentLength)
		buf := bytes.NewBuffer(bs)
		_, err := buf.ReadFrom(body)
		if err != nil {
			return nil, err
		}

		points, parseError := models.ParsePointsWithPrecision(buf.Bytes(), time.Now().UTC(), r.URL.Query().Get("precision"))
		if parseError != nil {
			return nil, parseError
		}

		i.pointsWritten += uint64(len(points))
		i.writes++
		i.lastLine = points

		return nil, nil
	}
	r := mux.NewRouter()
	r.HandleFunc("/write",
		n.MakeHTTPHandler(n.RestAPIFunc(testWriter))).Methods("POST")
	i.srv = &http.Server{Addr: url, Handler: r}
	i.url = url
	i.doneCh = make(chan bool)

	go func() {
		i.srv.ListenAndServe()
		close(i.doneCh)
	}()
}

func (i *testInfServer) Teardown() {
	i.srv.Close()
	<-i.doneCh
}

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
	s := testInfServer{}
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

	tu.Assert(t, s.writes == 1, "Expected 1 write got ", s.writes)
	tu.Assert(t, s.pointsWritten == 1, "Expected 1 point got ", s.pointsWritten)

	// write a batch of points
	p = getPoints(ss, 10)
	c.WritePoints(testDB, testMeas, p)
	tu.Assert(t, s.writes == 2, "Expected 2 write got ", s.writes)
	tu.Assert(t, s.pointsWritten == 11, "Expected 11 point got ", s.pointsWritten)
	tu.Assert(t, len(s.lastLine) == 10, "Expected 10 points got ", len(s.lastLine))
}

func TestCollectorDual(t *testing.T) {
	// start two servers
	s1 := testInfServer{}
	s1.Setup(portA)

	s2 := testInfServer{}
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

	tu.Assert(t, s1.writes == 1, "Expected 1 write got ", s1.writes)
	tu.Assert(t, s2.writes == 1, "Expected 1 write got ", s2.writes)
	tu.Assert(t, reflect.DeepEqual(s1.lastLine, s2.lastLine), "lastLines did not match")

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
	tu.Assert(t, s1.writes == 3, "Expected 3 writes got ", s1.writes)
	tu.Assert(t, s1.pointsWritten == 19, "Expected 19 points got ", s1.pointsWritten)
	tu.Assert(t, len(s1.lastLine) == 6, "Expected 6 points got ", len(s1.lastLine))
	tu.Assert(t, s2.writes == 3, "Expected 3 writes got ", s2.writes)
	tu.Assert(t, s2.pointsWritten == 19, "Expected 19 points got ", s2.pointsWritten)
	tu.Assert(t, len(s2.lastLine) == 6, "Expected 6 points got ", len(s2.lastLine))
	tu.Assert(t, reflect.DeepEqual(s1.lastLine, s2.lastLine), "lastLines did not match")
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
	tu.Assert(t, s2.writes == 5, "Expected 5 writes got ", s2.writes)
	tu.Assert(t, s2.pointsWritten == 37, "Expected 37 points got ", s2.pointsWritten)
	tu.Assert(t, len(s2.lastLine) == 6, "Expected 6 points got ", len(s2.lastLine))
	tu.Assert(t, s1.writes == 3, "Expected 3 writes got ", s1.writes)
}
