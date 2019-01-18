// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"fmt"
	"runtime"

	"github.com/pensando/sw/api/generated/metrics_query"
	"github.com/pensando/sw/venice/utils/metricsclient"

	"github.com/pensando/sw/venice/globals"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/proto/goproto"
	"github.com/pensando/sw/venice/utils/ntranslate"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type keylookup struct {
	version string
}

func (m *keylookup) KeyToMeta(key interface{}) *api.ObjectMeta {
	return &api.ObjectMeta{
		Name:      fmt.Sprintf("%d", key),
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}
}

func (m *keylookup) MetaToKey(meta *api.ObjectMeta) interface{} {
	return nil
}

func (it *veniceIntegSuite) TestMetrics(c *C) {
	// metrics iterators don't work in OSX
	if runtime.GOOS == "darwin" {
		return
	}

	tslt := ntranslate.MustGetTranslator()
	Assert(c, tslt != nil, "failed to get translator")
	tslt.Register("LifMetrics", &keylookup{})
	iter, err := goproto.NewLifMetricsIterator()
	AssertOk(c, err, "Error creating metrics iterator")

	// create an entry
	tmtr, err := iter.Create(3000)
	AssertOk(c, err, "Error creating test metrics entry")

	// set some values
	tmtr.SetRxBroadcastBytes(200)
	tmtr.SetRxBroadcastPackets(300)
	tmtr.SetRxCsumComplete(400.0)
	tmtr.SetRxDescFetchError(500.0)

	// query
	apiGwAddr := "localhost:" + it.config.APIGatewayPort
	mc, err := metricsclient.NewMetricsClient(apiGwAddr)
	AssertOk(c, err, "Error creating metrics client")

	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error in logged in context")

	AssertEventually(c, func() (bool, interface{}) {
		nodeQuery := &metrics_query.QueryList{
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			Queries: []*metrics_query.QuerySpec{
				{
					TypeMeta: api.TypeMeta{
						Kind: "LifMetrics",
					},
				},
			},
		}

		res, err := mc.Query(ctx, nodeQuery)
		if err != nil {
			return false, err
		}

		if len(res.Results) == 0 || len(res.Results[0].Series) == 0 {
			return false, res
		}

		return true, res

	}, "failed to query metrics", "5s", "180s")
}
