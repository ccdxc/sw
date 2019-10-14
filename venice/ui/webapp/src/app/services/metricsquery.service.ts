import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { ControllerService } from '@app/services/controller.service';
import { TelemetryqueryService as TelemetryqueryServiceGen } from '@app/services/generated/telemetryquery.service';
import { PollingInstance, PollUtility } from '@app/services/PollUtility';
import { ITelemetry_queryMetricsQuerySpec, Telemetry_queryMetricsQuerySpec, Telemetry_queryMetricsQueryList, Telemetry_queryMetricsQueryResult, ITelemetry_queryMetricsQueryList } from '@sdk/v1/models/generated/telemetry_query';

import { ITelemetry_queryMetricsQueryResponse, Telemetry_queryMetricsQueryResponse, ITelemetry_queryMetricsQueryResult } from '@sdk/v1/models/telemetry_query';
import { UIConfigsService } from './uiconfigs.service';

export interface MetricsPollingOptions {
  timeUpdater?: (body: ITelemetry_queryMetricsQuerySpec) => void;
  mergeFunction?: (curr: ITelemetry_queryMetricsQueryResult, newData: ITelemetry_queryMetricsQueryResult) => ITelemetry_queryMetricsQueryResult;
}

export interface MetricsPollingQuery {
  query: ITelemetry_queryMetricsQuerySpec;
  pollingOptions?: MetricsPollingOptions;
}

export interface TelemetryPollingMetricQueries {
  queries: MetricsPollingQuery[];
  tenant: string;
}

/**
 * Metrics Query Polling
 *
 * To allow for multiple components to perform polls with different bodies,
 * each poll request is associated with a key provided by the consumer.
 * Each poll request will automatically be updated with a time selector to only
 * fetch new items. If the user makes another call to pollEvents with a different body,
 * the poll will reset and be called with the new body. It will use the same data handler.
 *
 */

@Injectable()
export class MetricsqueryService extends TelemetryqueryServiceGen {
  pollingUtility: PollUtility;
  // Metrics are reported every 30 seconds in the backend
  POLLING_INTERVAL = 30000;
  metrics: Array<any> = [];

  constructor(protected _http: HttpClient,
    protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService) {
    super(_http, _controllerService, uiconfigsService);
    this.pollingUtility = new PollUtility(
      (key, body, useRealData) => {
        this.pollingFetchData(key, body, useRealData);
      },
      this.POLLING_INTERVAL);
  }

  /**
   * Takes in a body for the post query, as well as a timeUpdater
   * function which will be called to modify the request body, and
   * mergeFunction which is called to merge the new data with the old
   */
  pollMetrics(key: string, body: TelemetryPollingMetricQueries, pollingInterval = this.POLLING_INTERVAL) {
    const poll: PollingInstance = this.pollingUtility.initiatePolling(key, body, true, pollingInterval, 0, new Telemetry_queryMetricsQueryResponse());
    return poll.handler;
  }

  /**
   * Called when the timer ticks to fetch data
   */
  protected pollingFetchData(key, body: TelemetryPollingMetricQueries, useRealData): void {
    // remove previous poll if its there
    this.pollingUtility.pollingHandlerMap[key].pollSub.forEach( s => {
      s.unsubscribe();
    });
    this.pollingUtility.pollingHandlerMap[key].pollSub = [];

    const _ = Utility.getLodash();

    const queries: Telemetry_queryMetricsQuerySpec[] = [];
    const pollingOptions: MetricsPollingOptions[] = [];
    body.queries.forEach((query) => {
      queries.push(Utility.TrimDefaultsAndEmptyFields(new Telemetry_queryMetricsQuerySpec(query.query)));
      const pollOptions = query.pollingOptions || {};
      pollingOptions.push(pollOptions);
    });
    const queryList = new Telemetry_queryMetricsQueryList();
    queryList.queries = queries.map((q) => q.getModelValues());

    const sub = this.PostMetrics(queryList).subscribe(
      (resp) => {
        const respBody = new Telemetry_queryMetricsQueryResponse(resp.body as any);

        const poll = this.pollingUtility.pollingHandlerMap[key];
        const pollBody: TelemetryPollingMetricQueries = poll.body;
        if (poll == null) {
          return;
        }
        const currVal: ITelemetry_queryMetricsQueryResponse = poll.handler.value;
        const resValue: ITelemetry_queryMetricsQueryResponse = _.cloneDeep(currVal);
        resValue.tenant = respBody.tenant;

        respBody.results.forEach((r, i) => {
          const options = pollingOptions[i];
          // Influx may rename columns, so we set them
          // back to what they were in the query.
          if (r.series != null) {
            r.series.forEach((s) => {
              pollBody.queries[i].query.fields.forEach((field, ii) => {
                // First column is always time
                s.columns[ii + 1] = field;
              });
            });
          }

          if (poll.isFirstPoll) {
            // Don't append data
            resValue.results[i] = respBody.results[i];
            poll.isFirstPoll = false;
            // Modify time selector
            if (options.timeUpdater != null) {
              options.timeUpdater(pollBody.queries[i].query);
            }
          } else {
            // We assume that the incoming poll is only for new data

            // If no merge function is specified, we only
            // pass in the new data into the handler.
            if (options.mergeFunction != null) {
              resValue.results[i] = options.mergeFunction(resValue.results[i], respBody.results[i]);
            } else {
              resValue.results[i] = respBody.results[i];
            }

            if (options.timeUpdater != null) {
              options.timeUpdater(pollBody.queries[i].query);
            }
          }
        });

        this.pollingUtility.pollingHandlerMap[key].handler.next(resValue);

      },
      (error) => {
        this._controllerService.invokeRESTErrorToaster('Failed to get metrics', error);
        const poll = this.pollingUtility.pollingHandlerMap[key];
        if (poll == null || poll.handler == null) {
          return;
        }
        poll.handler.error(error);
      }
    );
    this.pollingUtility.pollingHandlerMap[key].pollSub.push(sub);
  }

  // If a query requests data for a slice of time where there are
  // no entries, DB returns null values. This function fitlers these values out.
  processData(data: ITelemetry_queryMetricsQueryResponse) {
    data.results.forEach((r) => {
      if (r.series == null) {
        return;
      }
      r.series.forEach((s) => {
        s.values = Utility.getLodash().filter(s.values, item => {
          if (item == null || item[1] == null) {
            // item has all null values
            return false;
          }
          return true;
        });
      });
    });

    let i = 1;
    while (i < data.results.length) {
      const prevId = data.results[i - 1].statement_id;
      const currId = data.results[i].statement_id;
      if (currId === prevId) {
        // we combine the two results into a series
        data.results[i - 1].series.push(...data.results[i].series);
        data.results.splice(i, 1);
        // since we spliced the array, we don't increment i
      } else {
        i++;
      }
    }
  }


}
