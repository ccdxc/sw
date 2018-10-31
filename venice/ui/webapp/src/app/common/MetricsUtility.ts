import { IMetrics_queryQuerySpec, Metrics_queryQuerySpec_function, Metrics_queryQuerySpec, LabelsRequirement_operator, ILabelsSelector } from '@sdk/v1/models/generated/metrics_query';
import { Utility } from './Utility';
import { IMetrics_queryQueryResponse } from '@sdk/v1/models/metrics_query';

export class MetricsUtility {

  public static timeSeriesQuery(kind, selector: ILabelsSelector = null): Metrics_queryQuerySpec {
    const timeSeriesQuery: IMetrics_queryQuerySpec = {
      'kind': kind,
      'meta': {
        'tenant': Utility.getInstance().getTenant(),
        'selector': selector
      },
      function: Metrics_queryQuerySpec_function.MEAN,
      'group-by-time': '5m',
      // We don't specify the fields we need, as specifying more than one field
      // while using the average function isn't supported by the backend.
      // Instead we leave blank and get all fields
      fields: [],
      'start-time': 'now() - 24h' as any,
      // Round down so we don't pick up an incomplete bucket
      'end-time': Utility.roundDownTime(5).toISOString() as any,
    };

    return new Metrics_queryQuerySpec(timeSeriesQuery);
  }

  // Since we are averaging over 5 min buckets, we always query from the last 5 min window increment
  public static timeSeriesQueryUpdate(queryBody) {
    queryBody['start-time'] = queryBody['end-time'];
    queryBody['end-time'] = Utility.roundDownTime(5).toISOString() as any;
  }

  public static timeSeriesQueryMerge(currValue, newData) {
    // Drops any values that are older than 24 hours from the current time.
    // We then add on the newer values.
    const _ = Utility.getLodash();
    const window = 24 * 60;
    // If window is positive, we filter any items that
    // have a timestamp less than Now - window
    if (window !== -1) {
      const moment = Utility.getMomentJS();
      const windowStart = moment().subtract(window, 'minutes');
      const filteredValues = _.dropWhile(currValue.results[0].series[0].values, (item) => {
        // Assuming time is the first index.
        return windowStart.diff(moment(item[0]), 'minutes') > 0;
      });
      currValue.results[0].series[0].values = _.cloneDeep(filteredValues);
    }

    // Checking if there is new data
    if (MetricsUtility.hasData(newData)) {
      const data = newData.results[0].series[0].values;
      // Push on the new data
      currValue.results[0].series[0].values.push(...data);
    }
    return currValue;
  }



  public static pastDayAverageQuery(kind, selector = null): Metrics_queryQuerySpec {
    const avgQuery: IMetrics_queryQuerySpec = {
      'kind': 'Node',
      'meta': {
        'tenant': Utility.getInstance().getTenant(),
        'selector': selector
      },
      function: Metrics_queryQuerySpec_function.MEAN,
      // We don't specify the fields we need, as specifying more than one field
      // while using the average function isn't supported by the backend.
      // Instead we leave blank and get all fields
      fields: [],
      'start-time': 'now() - 24h' as any,
      'end-time': 'now()' as any,
    };

    return new Metrics_queryQuerySpec(avgQuery);
  }

  public static pastDayAverageQueryUpdate(queryBody: IMetrics_queryQuerySpec) {
    queryBody['start-time'] = 'now() - 24h' as any;
    queryBody['end-time'] = 'now()' as any;
  }


  public static hasData(resp: IMetrics_queryQueryResponse) {
    if (resp && resp.results && resp.results.length !== 0 && resp.results[0].series && resp.results[0].series.length !== 0 && resp.results[0].series[0].values.length !== 0) {
      return true;
    } else {
      return false;
    }
  }

  public static createNameSelector(name: string): ILabelsSelector {
    if (name != null) {
      return {
        'requirements': [
          {
            'key': 'Name',
            'operator': LabelsRequirement_operator.equals,
            'values': [name]
          }
        ]
      };
    }
  }


}
