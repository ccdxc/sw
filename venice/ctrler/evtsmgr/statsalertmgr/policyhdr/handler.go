package policyhdr

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/statsalertmgr/alertengine"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/statsalertmgr/types"
	"github.com/pensando/sw/venice/utils/log"
)

//
// StatsPolicyHandler is responsible for running the metrics query (from the underlying stats alert policy)
// in intervals and sending the query response to alert engine for further action. The query results are currently
// grouped by a measurement function by default.
//
// TODO:
// 1. For now, all the threshold values are being converted to float64 by default, this conversion
//    needs to happen based on the metric type.
//

// StatsPolicyHandler represents the policy handler for a stats alert policy.
type StatsPolicyHandler struct {
	polName          string                            // name of the stats alert policy that this handler is dealing with
	statsAlertPolicy *monitoring.StatsAlertPolicy      // policy that this handler is dealing with
	pollInterval     time.Duration                     // metrics poll interval
	metricsClient    telemetry_query.TelemetryV1Client // metrics/telemetry query service client
	alertEngine      *alertengine.StatsAlertEngine     // stats alert engine that creates/updates alerts based on query response
	logger           log.Logger                        // logger
	ctx              context.Context                   // context to cancel the policy handler (goroutine)
	cancelFunc       context.CancelFunc                // context to cancel the policy handler (goroutine)
	wg               sync.WaitGroup
}

var (
	defaultQueryTimeout = 10 * time.Second

	// for queries on instantaneous values (when the measurement criteria is nil)
	defaultMeasurementWindow = "60s"
	defaultPollInterval      = "60s" // to not overload citadel every 30s, +30s buffer is added for polling
)

// Option fills the optional params for StatsPolicyHandler
type Option func(*StatsPolicyHandler)

// WithPollInterval passes a custom poll interval to the handler.
func WithPollInterval(pollInterval time.Duration) Option {
	return func(ph *StatsPolicyHandler) {
		ph.pollInterval = pollInterval
	}
}

// NewStatsPolicyHandler creates a new stats policy handler for the given alert policy.
func NewStatsPolicyHandler(parentCtx context.Context, polName string, sap *monitoring.StatsAlertPolicy, metricClient telemetry_query.TelemetryV1Client,
	alertEngine *alertengine.StatsAlertEngine, logger log.Logger, opts ...Option) (*StatsPolicyHandler, error) {

	pollIntervalInStr := defaultPollInterval // it has to be a query on the instantaneous value
	if sap.Spec.MeasurementCriteria != nil {
		pollIntervalInStr = sap.Spec.MeasurementCriteria.Window
	}

	pollInterval, err := time.ParseDuration(pollIntervalInStr)
	if err != nil {
		logger.Errorf("failed to parse measurement window from policy: {%s, %s}, err: %v", sap.GetName(), pollInterval, err)
		return nil, err
	}

	ctx, cancelFunc := context.WithCancel(parentCtx)
	ph := &StatsPolicyHandler{
		polName:          polName,
		statsAlertPolicy: sap,
		pollInterval:     pollInterval,
		metricsClient:    metricClient,
		alertEngine:      alertEngine,
		logger:           logger,
		ctx:              ctx,
		cancelFunc:       cancelFunc,
	}

	for _, opt := range opts {
		if opt != nil {
			opt(ph)
		}
	}

	return ph, nil
}

// Start starts the policy handler that converts the policy to query; runs the query against citadel in regular
// intervals.
func (p *StatsPolicyHandler) Start() {
	go func() {
		p.wg.Add(1)
		p.start()
	}()
}

// Stop stops the policy handler.
func (p *StatsPolicyHandler) Stop() {
	p.cancelFunc()
	p.wg.Wait()
}

// Name returns the name of the policy handler (policyName/policyUUID).
func (p *StatsPolicyHandler) Name() string {
	return p.polName
}

// helper function that converts the policy to query and runs it against citadel.
func (p *StatsPolicyHandler) start() {
	defer p.wg.Done()

	ticker := time.NewTicker(p.pollInterval)
	defer ticker.Stop()

	for {
		select {
		case <-p.ctx.Done():
			p.logger.Infof("{%s} context cancelled; exiting from policy handler for policy", p.polName)
			return
		case <-ticker.C:
			metricsQuery := StatsAlertPolicyToCitadelQuery(*p.statsAlertPolicy)
			p.logger.Infof("{%s} time to execute the metrics query: %v", p.polName, metricsQuery)

			// run the metrics query
			ctx, cancelFunc := context.WithTimeout(p.ctx, defaultQueryTimeout)
			metricsQueryResp, err := p.metricsClient.Metrics(ctx, &metricsQuery)
			cancelFunc()
			if err != nil {
				log.Errorf("{%s} failed to query, err: %v", p.polName, err)
				continue
			}

			// parse query response
			parsedResp, err := p.parseMetricsQueryResponse(metricsQueryResp)
			if err != nil {
				p.logger.Errorf("{%s} failed to parse metrics query, err: %v", p.polName, err)
			}

			// send the response to alert engine
			if err := p.alertEngine.ProcessQueryResponse(p.statsAlertPolicy.GetObjectMeta(), parsedResp); err != nil {
				p.logger.Errorf("{%s} alert engine failed to process query response, err: %v", p.polName, err)
			}
		}
	}
}

// helper function that parses the metrics query response to types.QueryResponse.
func (p *StatsPolicyHandler) parseMetricsQueryResponse(resp *telemetry_query.MetricsQueryResponse) (
	*types.QueryResponse, error) {
	parsedResp := &types.QueryResponse{ByReporterID: make(map[string]*types.MetricValue)}
	if resp != nil {
		for _, res := range resp.Results {
			for _, series := range res.Series {
				if _, found := series.Tags["reporterID"]; !found {
					return nil, fmt.Errorf("{%s} reporter ID not found in the metrics response", p.polName)
				}

				if len(series.Columns) > 2 {
					return nil, fmt.Errorf("{%s} received more than one data point (cols)", p.polName)
				}

				reporterID := series.Tags["reporterID"]
				parsedResp.ByReporterID[reporterID] = &types.MetricValue{}

				for index, col := range series.Columns {
					if col == "time" {
						parsedResp.ByReporterID[reporterID].Time = series.Values[0].Values[index].GetStr()
					} else {
						switch val := series.Values[0].Values[index].Value.(type) {
						case *api.Interface_Float:
							parsedResp.ByReporterID[reporterID].Val = val.Float
						case *api.Interface_Int64:
							parsedResp.ByReporterID[reporterID].Val = float64(val.Int64)
						default:
							return nil, fmt.Errorf("{%s} invalid metric value received", p.polName)
						}
					}
				}
			}
		}
	}
	return parsedResp, nil
}
