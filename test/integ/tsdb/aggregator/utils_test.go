package aggregatorinteg

import (
	"net"
	"net/http"
	"os"
	"sync"

	"github.com/influxdata/influxdb/services/httpd"
	"github.com/influxdata/influxdb/services/meta"

	"github.com/pensando/sw/test/integ/tsdb/aggutils"
	"github.com/pensando/sw/venice/aggregator/gatherer"
	"github.com/pensando/sw/venice/aggregator/server"
)

type Agg struct {
	mc  *meta.Client
	lis net.Listener
	wg  sync.WaitGroup
}

const metaDir = "/tmp/agg-meta"

func StartAgg(beURLs []string, listenURL string) (*Agg, error) {
	err := gatherer.SetBackends(beURLs)
	if err != nil {
		return nil, err
	}

	if err := os.MkdirAll(metaDir, 0777); err != nil {
		return nil, err
	}

	mc := meta.NewClient(&meta.Config{Dir: metaDir})
	if err := mc.Open(); err != nil {
		return nil, err
	}

	_, err = mc.CreateDatabase("db0")
	if err != nil {
		return nil, err
	}

	if _, err := mc.CreateRetentionPolicy("db0", aggutils.NewRetentionPolicySpec("rp0", 1, 0), true); err != nil {
		return nil, err
	}

	config := httpd.Config{BindAddress: listenURL, Enabled: true}
	handler := server.NewHandler(config, mc)
	listener, err := net.Listen("tcp", listenURL)
	if err != nil {
		return nil, err
	}

	agg := &Agg{
		lis: listener,
		mc:  mc,
	}

	agg.wg.Add(1)
	go func() {
		http.Serve(listener, handler)
		agg.wg.Done()
	}()

	return agg, nil

}

func (a *Agg) Addr() string {
	return "http://" + a.lis.Addr().String()
}
func (a *Agg) Close() {
	a.lis.Close()
	a.wg.Wait()
	a.mc.Close()
	os.RemoveAll(metaDir)
}
