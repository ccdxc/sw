// +build apulu

package apulu

import (
	"context"
	"io"
	"time"

	"github.com/gogo/protobuf/proto"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pkg/errors"
	uuid "github.com/satori/go.uuid"
)

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
			metricsGetRequest.Name = "Port"
		case netproto.InterfaceSpec_UPLINK_MGMT.String():
			metricsGetRequest.Name = "MgmtPort"
		case netproto.InterfaceSpec_HOST_PF.String():
			metricsGetRequest.Name = "Lif"
		default:
			// statistics on other types of interfaces are not supported
			continue
		}
		log.Infof("Querying metrics for interface %s, %s", intf.UUID, intf.Name)
		err = stream.Send(metricsGetRequest)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrMetricsSend,
				"Error querying metris of intf %s | Err %v",
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
			log.Infof("Rcvd metrics for %s", uuid.FromBytesOrNil(resp.Response.Key).String())
			// TODO: export the metrics
		} else {
			log.Error(errors.Wrapf(types.ErrMetricsRecv,
				"Metrics response failure, | Err %v", resp.GetApiStatus().String()))
			continue
		}
	}
	return nil
}

func HandleMetrics(infraAPI types.InfraAPI, client halapi.OperSvcClient) error {
	// create a bidir stream for metrics
	metricsStream, err := client.MetricsGet(context.Background())
	if err != nil {
		log.Error(errors.Wrapf(types.ErrMetricsGet, "MetricsGet failure | Err %v", err))
		return errors.Wrapf(types.ErrMetricsGet, "MetricsGet failure | Err %v", err)
	}

	// periodically query for metrics from PDS agent
	go func(stream halapi.OperSvc_MetricsGetClient) {
		ticker := time.NewTicker(time.Minute * 3)
		for {
			select {
			case <-ticker.C:
				log.Infof("Querying Interface metrics")
				queryInterfaceMetrics(infraAPI, stream)
			}
		}
	}(metricsStream)
	return nil
}
