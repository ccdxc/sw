package collector

import (
	"sync"

	influx "github.com/influxdata/influxdb/client/v2"
)

const maxTries = 3

// Implementation of Influx backend for the collector
type infClient struct {
	sync.WaitGroup
	c     influx.Client
	errCh chan<- error
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
