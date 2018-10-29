import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { ControllerService } from '@app/services/controller.service';
import { MetricsqueryService as MetricsqueryGenService } from '@app/services/generated/metricsquery.service';
import { PollingInstance, PollUtility } from '@app/services/PollUtility';
import { IMetrics_queryQuerySpec, Metrics_queryQuerySpec } from '@sdk/v1/models/generated/metrics_query';

import { IMetrics_queryQueryResponse, Metrics_queryQueryResponse } from '@sdk/v1/models/metrics_query';

export interface MetricsPollingOptions {
  timeUpdater?: (body: IMetrics_queryQuerySpec) => void;
  mergeFunction?: (curr: IMetrics_queryQueryResponse, newData: IMetrics_queryQueryResponse) => IMetrics_queryQueryResponse;
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
export class MetricsqueryService extends MetricsqueryGenService {
  pollingUtility: PollUtility;
  // Metrics are reported every 30 seconds in the backend
  POLLING_INTERVAL = 30000;
  metrics: Array<any> = [];
  pollingOptionsMap: { [key: string]: MetricsPollingOptions } = {};

  constructor(protected _http: HttpClient,
    protected _controllerService: ControllerService) {
    super(_http, _controllerService);
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
  pollMetrics(key: string, body: IMetrics_queryQuerySpec = {}, pollOptions: MetricsPollingOptions = null) {
    const poll: PollingInstance = this.pollingUtility.initiatePolling(key, body, true, this.POLLING_INTERVAL, 0, new Metrics_queryQueryResponse());
    this.pollingOptionsMap[key] = pollOptions;
    return poll.handler;
  }

  /**
   * Called when the timer ticks to fetch data
   */
  protected pollingFetchData(key, body, useRealData): void {
    const query = new Metrics_queryQuerySpec(body);
    this.PostQuery(query).subscribe(
      (resp) => {
        const options = this.pollingOptionsMap[key];
        const respBody = new Metrics_queryQueryResponse(resp.body as any);
        const poll = this.pollingUtility.pollingHandlerMap[key];
        if (poll == null) {
          return false;
        }
        this.processData(respBody);
        if (poll.isFirstPoll) {
          // Don't append data
          this.pollingUtility.pollingHandlerMap[key].handler.next(respBody);
          poll.isFirstPoll = false;
          // Modify time selector
          if (options.timeUpdater != null) {
            options.timeUpdater(poll.body);
          }
        } else {
          const _ = Utility.getLodash();
          // We assume that the incoming poll is only for new data
          const currVal = poll.handler.value;
          let resValue = _.cloneDeep(currVal);

          // If no merge function is specified, we only
          // pass in the new data into the handler.
          if (options.mergeFunction != null) {
            resValue = options.mergeFunction(resValue, respBody);
            this.pollingUtility.pollingHandlerMap[key].handler.next(resValue);
          } else {
            this.pollingUtility.pollingHandlerMap[key].handler.next(respBody);
          }

          if (options.timeUpdater != null) {
            options.timeUpdater(poll.body);
          }
        }
      },
      (error) => {
        console.error('Polling for metrics failed', error);
      }
    );
  }

  // If a query requests data for a slice of time where there are
  // no entries, DB returns null values. We fitler these values out.
  processData(data) {
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
  }

}
