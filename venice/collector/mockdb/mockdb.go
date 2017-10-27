package mockdb

import (
	"bytes"
	"fmt"
	"net/http"
	"time"

	"github.com/gorilla/mux"
	"github.com/influxdata/influxdb/models"

	"github.com/pensando/sw/venice/utils/log"
	n "github.com/pensando/sw/venice/utils/netutils"
)

// MockTSDB implements a mock server for testing
type MockTSDB struct {
	url           string
	srv           *http.Server
	doneCh        chan bool
	PointsWritten uint64
	Writes        uint64
	LastLine      []models.Point
}

// Setup starts a server at the specified url
func (i *MockTSDB) Setup(url string) {

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

// Teardown closes the server and waits until it exits
func (i *MockTSDB) Teardown() {
	i.srv.Close()
	<-i.doneCh
}
