package collector

import (
	"crypto/tls"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
	"sync"

	influx "github.com/influxdata/influxdb/client/v2"

	"github.com/pensando/sw/venice/utils/runtime"
)

const maxTries = 3

// Implementation of Influx backend for the collector
type infClient struct {
	sync.WaitGroup
	c          influx.Client
	errCh      chan<- error
	u          *url.URL
	httpClient *http.Client
}

// NewInfluxBE returns an instance of the influx backend
func NewInfluxBE() Backend {
	return &infClient{}
}

func (ic *infClient) Open(conf BEConfig, errCh chan<- error) error {
	hc := influx.HTTPConfig{
		Addr:     conf.Addr,
		Username: conf.Username,
		Password: conf.Password,
		Timeout:  conf.Timeout,
	}
	c, err := influx.NewHTTPClient(hc)
	if err != nil {
		return err
	}

	ic.c = c
	ic.errCh = errCh
	ic.u, _ = url.Parse(conf.Addr)
	tr := &http.Transport{
		TLSClientConfig: &tls.Config{},
	}
	ic.httpClient = &http.Client{
		Timeout:   conf.Timeout,
		Transport: tr,
	}

	return nil
}

// Close closes the client and waits for all pending go routines to finish
func (ic *infClient) Close() {
	ic.c.Close()
	ic.Wait()
	close(ic.errCh)
}

func (ic *infClient) Write(i interface{}, wg *sync.WaitGroup) {
	ic.Add(1)
	go ic.write(i, wg)
}

func (ic *infClient) write(i interface{}, wg *sync.WaitGroup) {
	defer ic.Done()
	defer wg.Done()
	var err error

	bp := i.(influx.BatchPoints)
	for ix := 0; ix < maxTries; ix++ {
		err = ic.c.Write(bp)
		if err == nil {
			return
		}
	}

	ic.errCh <- err
}

func (ic *infClient) WriteLines(db string, lines []string, wg *sync.WaitGroup) {
	ic.Add(1)
	go ic.writeLP(db, lines, wg)
}

func (ic *infClient) writeLP(db string, lines []string, wg *sync.WaitGroup) {
	defer ic.Done()
	defer wg.Done()
	var err error
	sr := runtime.NewStringsReader(lines)

	u := ic.u
	u.Path = "write"
	req, err := http.NewRequest("POST", u.String(), sr)
	if err != nil {
		ic.errCh <- err
		return
	}
	req.ContentLength = sr.Size()

	params := req.URL.Query()
	params.Set("db", db)
	params.Set("rp", "")
	params.Set("precision", "ns")
	req.URL.RawQuery = params.Encode()

	resp, err := ic.httpClient.Do(req)
	if err != nil {
		ic.errCh <- err
		return
	}
	defer resp.Body.Close()

	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		ic.errCh <- err
		return
	}

	if resp.StatusCode != http.StatusNoContent && resp.StatusCode != http.StatusOK {
		var err = fmt.Errorf(string(body))
		ic.errCh <- err
		return
	}

}
