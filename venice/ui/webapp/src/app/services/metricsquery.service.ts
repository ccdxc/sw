import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { ControllerService } from '@app/services/controller.service';
import { MetricsqueryService as MetricsqueryGenService } from '@app/services/generated/metricsquery.service';
import { PollingInstance, PollUtility } from '@app/services/PollUtility';
import { IMetrics_queryQuerySpec, Metrics_queryQuerySpec, Metrics_queryQueryList, Metrics_queryQueryResult } from '@sdk/v1/models/generated/metrics_query';

import { IMetrics_queryQueryResponse, Metrics_queryQueryResponse, IMetrics_queryQueryResult } from '@sdk/v1/models/metrics_query';

export interface MetricsPollingOptions {
  timeUpdater?: (body: IMetrics_queryQuerySpec) => void;
  mergeFunction?: (curr: IMetrics_queryQueryResult, newData: IMetrics_queryQueryResult) => IMetrics_queryQueryResult;
}

export interface MetricsPollingQuery {
  query: IMetrics_queryQuerySpec;
  pollingOptions?: MetricsPollingOptions;
}

export interface MetricsPollingQueries {
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
export class MetricsqueryService extends MetricsqueryGenService {
  pollingUtility: PollUtility;
  // Metrics are reported every 30 seconds in the backend
  POLLING_INTERVAL = 30000;
  metrics: Array<any> = [];

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
  pollMetrics(key: string, body: MetricsPollingQueries, pollingInterval = this.POLLING_INTERVAL) {
    const poll: PollingInstance = this.pollingUtility.initiatePolling(key, body, true, pollingInterval, 0, new Metrics_queryQueryResponse());
    return poll.handler;
  }

  /**
   * Called when the timer ticks to fetch data
   */
  protected pollingFetchData(key, body: MetricsPollingQueries, useRealData): void {
    const queries: Metrics_queryQuerySpec[] = [];
    const pollingOptions = [];
    body.queries.forEach((query) => {
      queries.push(new Metrics_queryQuerySpec(query.query));
      const pollOptions = query.pollingOptions || {};
      pollingOptions.push(pollOptions);
    });
    const queryList = new Metrics_queryQueryList();
    queryList.queries = queries.map((q) => q.getModelValues());
    this.PostQuery(queryList).subscribe(
      (resp) => {
        const respBody = new Metrics_queryQueryResponse(resp.body as any);
        const poll = this.pollingUtility.pollingHandlerMap[key];
        if (poll == null) {
          return false;
        }
        this.processData(respBody);
        respBody.results.forEach((r, i) => {
          const options = pollingOptions[i];
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

        });

      },
      (error) => {
        console.error('Polling for metrics failed', error);
        const poll = this.pollingUtility.pollingHandlerMap[key];
        if (poll == null) {
          return false;
        }
        poll.handler.error('Metrics query failed');
      }
    );
  }

  // If a query requests data for a slice of time where there are
  // no entries, DB returns null values. We fitler these values out.
  processData(data: IMetrics_queryQueryResponse) {
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
