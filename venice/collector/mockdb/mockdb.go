package mockdb

import (
	"bytes"
	"fmt"
	"net/http"
	"time"

	"github.com/influxdata/influxdb/models"

	"github.com/pensando/sw/venice/utils/log"
	n "github.com/pensando/sw/venice/utils/netutils"
	tu "github.com/pensando/sw/venice/utils/testutils"
)

// MockTSDB implements a mock server for testing
type MockTSDB struct {
	URL           string
	ms            *tu.MockServer
	PointsWritten uint64
	Writes        uint64
	LastLine      []models.Point
}

// Setup starts a server at the specified url
func (i *MockTSDB) Setup() (string, error) {
	models.EnableUintSupport() // temporary

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

		i.PointsWritten += uint64(len(points))
		i.Writes++
		i.LastLine = points

		return nil, nil
	}

	i.ms = tu.NewMockServer()
	i.ms.AddHandler("/write", "POST", n.MakeHTTPHandler(n.RestAPIFunc(testWriter)))
	go i.ms.Start()

	i.URL = i.ms.URL()
	log.Infof("server started {%+v}", i.URL)
	return i.URL, nil
}

// Teardown closes the server and waits until it exits
func (i *MockTSDB) Teardown() {
	i.ms.Stop()
	log.Infof("server closed {%+v}", i.URL)
}
