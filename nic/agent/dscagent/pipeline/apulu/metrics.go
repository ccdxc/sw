// +build apulu

package apulu

import (
	"context"
	"fmt"
	"io"
	"time"

	"github.com/gogo/protobuf/proto"
	"github.com/pkg/errors"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/tsdb"
)

const (
	metricsTablePort     = "MacMetrics"
	metricsTableMgmtPort = "MgmtMacMetrics"
	metricsTableHostIf   = "LifMetrics"
)

var metricsTables = []string{
	metricsTablePort,
	metricsTableMgmtPort,
	metricsTableHostIf,
}

var tsdbObjs map[string]tsdb.Obj

func queryInterfaceMetrics(infraAPI types.InfraAPI, stream halapi.OperSvc_MetricsGetClient) (err error) {
	var (
		dat [][]byte
	)

	dat, err = infraAPI.List("Interface")
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "Interface retrieval from db failed, Err: %v", err))
		return errors.Wrapf(types.ErrBadRequest, "Interface retrieval from db failed, Err: %v", err)
	}
	metricsGetRequest := &halapi.MetricsGetRequest{}

	// walk over all the interfaces and query for statistics
	for _, o := range dat {
		var intf netproto.Interface
		err := proto.Unmarshal(o, &intf)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err))
			continue
		}
		uid, _ := uuid.FromString(intf.UUID)
		metricsGetRequest.Key = uid.Bytes()
		switch intf.Spec.Type {
		case netproto.InterfaceSpec_UPLINK_ETH.String():
			metricsGetRequest.Name = metricsTablePort
		case netproto.InterfaceSpec_UPLINK_MGMT.String():
			metricsGetRequest.Name = metricsTableMgmtPort
		case netproto.InterfaceSpec_HOST_PF.String():
			metricsGetRequest.Name = metricsTableHostIf
		default:
			// statistics on other types of interfaces are not supported
			continue
		}
		//log.Infof("Querying metrics for interface %s, %s", intf.UUID, intf.Name)
		err = stream.Send(metricsGetRequest)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrMetricsSend,
				"Error querying metrics of intf %s, %s | Err %v",
				intf.UUID, intf.Name, err))
			continue
		}
		// process the response
		resp, err := stream.Recv()
		if err == io.EOF {
			break
		}
		if err != nil {
			log.Error(errors.Wrapf(types.ErrMetricsRecv,
				"Error receiving metrics | Err %v", err))
			continue
		}
		if resp.GetApiStatus() != halapi.ApiStatus_API_STATUS_OK {
			log.Error(errors.Wrapf(types.ErrMetricsRecv,
				"Metrics response failure, | Err %v", resp.GetApiStatus().String()))
			continue
		}
		//log.Infof("Rcvd metrics %v for intf %s, %s", metricsGetRequest.Name, intf.UUID, intf.Name)
		if _, ok := tsdbObjs[metricsGetRequest.Name]; !ok {
			log.Errorf("Ignoring unknown metrics %v", metricsGetRequest.Name)
			continue
		}

		// build the row to be added to tsdb
		points := []*tsdb.Point{}
		tags := map[string]string{"tenant": "", "name": intf.Name}
		fields := make(map[string]interface{})
		for _, row := range resp.Response {
			for _, counter := range row.Counters {
				fields[counter.Name] = counter.Value
			}
		}

		points = append(points, &tsdb.Point{Tags: tags, Fields: fields})
		if err := tsdbObjs[metricsGetRequest.Name].Points(points, time.Now()); err != nil {
			log.Errorf("failed to send metricd %v", err)
		}
	}
	return nil
}

// HandleMetrics handles collecting and reporting of metrics
func HandleMetrics(infraAPI types.InfraAPI, client halapi.OperSvcClient) error {
	// create a bidir stream for metrics
	metricsStream, err := client.MetricsGet(context.Background())
	if err != nil {
		log.Error(errors.Wrapf(types.ErrMetricsGet, "MetricsGet failure | Err %v", err))
		return errors.Wrapf(types.ErrMetricsGet, "MetricsGet failure | Err %v", err)
	}

	// periodically query for metrics from PDS agent
	go func(stream halapi.OperSvc_MetricsGetClient) {
		for {
			if ok := tsdb.IsInitialized(); ok {
				break
			}
			log.Infof("waiting to init tsdb")
			time.Sleep(time.Second * 2)
		}

		// create tsdb objects for all tables
		tsdbObjs = map[string]tsdb.Obj{}

		for _, table := range metricsTables {
			obj, err := tsdb.NewObj(table, nil, nil, nil)
			if err != nil {
				log.Errorf("Failed to create tsdb object for table %s", table)
				continue
			}

			if obj == nil {
				log.Errorf("Found invalid tsdb object for table %s", table)
				continue
			}
			tsdbObjs[table] = obj
		}

		ticker := time.NewTicker(time.Minute * 1)

		for {
			select {
			case <-ticker.C:
				log.Info("Querying Interface metrics")
				pdsAgentURL := fmt.Sprintf("127.0.0.1:%s", types.PDSGRPCDefaultPort)
				if utils.IsHalUp(pdsAgentURL) == false {
					// HAL is not up, skip querying
					continue
				}
				queryInterfaceMetrics(infraAPI, stream)
			}
		}
	}(metricsStream)
	return nil
}
