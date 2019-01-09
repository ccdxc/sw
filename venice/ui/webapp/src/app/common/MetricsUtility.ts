import { IMetrics_queryQuerySpec, Metrics_queryQuerySpec_function, Metrics_queryQuerySpec, LabelsRequirement_operator, ILabelsSelector } from '@sdk/v1/models/generated/metrics_query';
import { Utility } from './Utility';
import { IMetrics_queryQueryResponse, IMetrics_queryQueryResult } from '@sdk/v1/models/metrics_query';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { HeroCardOptions } from '@app/components/shared/herocard/herocard.component';
import { CardStates, StatArrowDirection } from '@app/components/shared/basecard/basecard.component';

/**
 * serverData is in the following form:
 * "results" [
 *   "series": [
        {
          "name": "Node",
          "columns": [
            "time",
            "CPUUsedPercent",
            "Kind",
            "MemAvailable",
            "MemFree",
            "MemTotal",
            "MemUsed",
            "MemUsedPercent",
            "Name",
            "reporterID"
          ],
          "values": [
            [
              "2018-10-04T01:40:16.71348949Z",
              72.9289625253088,
              "Node",
              13900562432,
              1176416256,
              16826892288,
              2942144512,
              17.484776521082107,
              "node1",
              "Node-node1"
            ],
            [
              "2018-10-04T01:42:52.780420968Z",
              69.15385763317215,
              "Node",
              12755824640,
              1896194048,
              16826892288,
              4062240768,
              24.141360736569066,
              "node2",
              "Node-node2"
            ],
            ...
  *
  * If it is grouped by a field then it is of the following form:
    "results": [
        {
            "statement_id": 0,
            "series": [
                {
                    "name": "Node",
                    "tags": { // Grouped field is here
                        "reporterID": "Node-node1"
                    },
                    "columns": [
                        "time",
                        ...
                    ],
                    "values": [
                        [
                            "2018-10-31T21:24:57.000000001Z",
                            ...
                        ]
                    ]
                }
            ]
        },
        {
            "statement_id": 0,
            "series": [
                {
                    "name": "Node",
                    "tags": {
                        "reporterID": "Node-node2"
                    },
                    "columns": [
                        "time",
                        ...
                    ],
                    "values": [
                        [
                            "2018-10-31T21:24:57.000000001Z",
                            ...
                        ]
                    ]
                }
            ]
        }
      ]
    }
  */
export class MetricsUtility {

  public static getStatArrowDirection(prev: number, curr: number): StatArrowDirection {
    if (prev < curr) {
      return StatArrowDirection.UP;
    } else if (prev > curr) {
      return StatArrowDirection.DOWN;
    } else {
      return StatArrowDirection.HIDDEN;
    }
  }

  public static timeSeriesQuery(kind: string, selector: ILabelsSelector = null): Metrics_queryQuerySpec {
    const timeSeriesQuery: IMetrics_queryQuerySpec = {
      'kind': kind,
      'selector': selector,
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
  public static timeSeriesQueryUpdate(queryBody: IMetrics_queryQuerySpec) {
    queryBody['start-time'] = queryBody['end-time'];
    queryBody['end-time'] = Utility.roundDownTime(5).toISOString() as any;
  }

  public static timeSeriesQueryMerge(currData: IMetrics_queryQueryResult, newData: IMetrics_queryQueryResult) {
    // Drops any values that are older than 24 hours from the current time.
    // We then add on the newer values.
    const _ = Utility.getLodash();
    const window = 24 * 60;
    if (MetricsUtility.resultHasData(currData)) {
      const moment = Utility.getMomentJS();
      const windowStart = moment().subtract(window, 'minutes');
      const filteredValues = _.dropWhile(currData.series[0].values, (item) => {
        // Assuming time is the first index.
        return windowStart.diff(moment(item[0]), 'minutes') > 0;
      });
      currData.series[0].values = _.cloneDeep(filteredValues);
    }

    // Checking if there is new data
    if (MetricsUtility.resultHasData(newData)) {
      const data = newData.series[0].values;
      if (MetricsUtility.resultHasData(currData)) {
        // Push on the new data
        currData.series[0].values.push(...data);
      } else {
        return newData;
      }
    }
    return currData;
  }

  public static pastFiveMinAverageQuery(kind: string, selector: ILabelsSelector = null): Metrics_queryQuerySpec {
    const avgQuery: IMetrics_queryQuerySpec = {
      'kind': kind,
      'selector': selector,
      function: Metrics_queryQuerySpec_function.MEAN,
      // We don't specify the fields we need, as specifying more than one field
      // while using the average function isn't supported by the backend.
      // Instead we leave blank and get all fields
      fields: [],
      'start-time': 'now() - 5m' as any,
      'end-time': 'now()' as any,
    };

    return new Metrics_queryQuerySpec(avgQuery);
  }

  public static pastFiveMinQueryUpdate(queryBody: IMetrics_queryQuerySpec) {
    queryBody['start-time'] = 'now() - 5m' as any;
    queryBody['end-time'] = 'now()' as any;
  }

  public static intervalAverageQuery(kind: string, startTime, endTime, selector: ILabelsSelector = null): Metrics_queryQuerySpec {
    const avgQuery: IMetrics_queryQuerySpec = {
      'kind': kind,
      'selector': selector,
      function: Metrics_queryQuerySpec_function.MEAN,
      // We don't specify the fields we need, as specifying more than one field
      // while using the average function isn't supported by the backend.
      // Instead we leave blank and get all fields
      fields: [],
      'start-time': startTime,
      'end-time': endTime
    };

    return new Metrics_queryQuerySpec(avgQuery);
  }

  public static genIntervalAverageQueryUpdate(startTime, endTime) {
    return (queryBody: IMetrics_queryQuerySpec) => {
      queryBody['start-time'] = startTime;
      queryBody['end-time'] = endTime;
    };
  }


  public static pastDayAverageQuery(kind: string, selector: ILabelsSelector = null): Metrics_queryQuerySpec {
    const avgQuery: IMetrics_queryQuerySpec = {
      'kind': kind,
      'selector': selector,
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

  public static maxObjQuery(kind: string, selector: ILabelsSelector = null): Metrics_queryQuerySpec {
    const maxNodeQuery: IMetrics_queryQuerySpec = {
      'kind': kind,
      'selector': selector,
      function: Metrics_queryQuerySpec_function.MEAN,
      // We don't specify the fields we need, as specifying more than one field
      // while using the average function isn't supported by the backend.
      // Instead we leave blank and get all fields
      fields: [],
      'group-by-field': 'reporterID',
      // We only look at the last 5 min bucket so that the max node reporting is never
      // lower than current
      'start-time': new Date(Utility.roundDownTime(5).getTime() - 1000 * 50 * 5).toISOString() as any,
      'end-time': Utility.roundDownTime(5).toISOString() as any
    };
    return new Metrics_queryQuerySpec(maxNodeQuery);
  }

  public static maxObjQueryUpdate(queryBody: IMetrics_queryQuerySpec) {
    queryBody['start-time'] = new Date(Utility.roundDownTime(5).getTime() - 1000 * 50 * 5).toISOString() as any,
      queryBody['end-time'] = Utility.roundDownTime(5).toISOString() as any;
  }

  public static maxObjQueryMerge(currData: IMetrics_queryQueryResult, newData: IMetrics_queryQueryResult) {
    // since we round down to get the last 5 min bucket, there's a chance
    // that we can back null data, since no new metrics have been reported.
    // Data should have been filtered in metricsquery services's processData
    if (!MetricsUtility.resultHasData(newData)) {
      // no new data, keep old value
      return currData;
    }
    return newData;
  }

  /**
   * Returns the name and avg of the node with the highest avg
   */
  public static maxObjUtility(data: IMetrics_queryQueryResult, fieldName: string): { name: string, max: number } {
    if (!MetricsUtility.resultHasData(data) || fieldName == null || fieldName === '') {
      return null;
    }
    const objMaxList = [];
    data.series.forEach((s) => {
      const index = s.columns.indexOf(fieldName);
      const tags = s.tags as any;
      const key = tags.reporterID;
      let max;
      if (index !== -1 &&
        index < s.values[0].length &&
        key != null &&
        s.values != null &&
        s.values.length !== 0) {
        max = s.values[0][index];
        objMaxList.push({ name: key, max: max });
      }
    });
    if (objMaxList.length === 0) {
      return null;
    }
    const maxObj = Utility.getLodash().maxBy(objMaxList, 'max');
    return maxObj;
  }

  public static responseHasData(resp: IMetrics_queryQueryResponse) {
    if (resp && resp.results &&
      resp.results.length !== 0 &&
      this.resultHasData(resp.results[0])) {
      return true;
    } else {
      return false;
    }
  }

  public static resultHasData(resp: IMetrics_queryQueryResult) {
    if (resp && resp.series &&
      resp.series.length !== 0 &&
      resp.series[0].values &&
      resp.series[0].values.length !== 0) {
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

  public static clusterLevelCPUHeroCard(themeColor: string, icon: Icon): HeroCardOptions {
    return {
      title: 'CPU',
      firstStat: {
        value: null,
        description: 'CPU Usage',
        tooltip: 'Averaged over past 5m'
      },
      secondStat: {
        value: null,
        description: '24h Avg',
        tooltip: 'Averaged over past 24h'
      },
      thirdStat: {
        value: null,
        description: 'Highest CPU Usage',
        tooltip: 'Averaged over past 5m'
      },
      data: {
        x: [],
        y: []
      },
      backgroundIcon: {
        svgIcon: 'cpu',
        margin: {}
      },
      themeColor: themeColor,
      icon: icon,
      cardState: CardStates.LOADING
    };
  }

  public static clusterLevelMemHeroCard(themeColor: string, icon: Icon): HeroCardOptions {
    return {
      title: 'Memory',
      firstStat: {
        value: null,
        description: 'Memory Usage',
        tooltip: 'Averaged over past 5m'
      },
      secondStat: {
        value: null,
        description: '24h Avg',
        tooltip: 'Averaged over past 24h'
      },
      thirdStat: {
        value: null,
        description: 'Highest Memory Usage',
        tooltip: 'Averaged over past 5m'
      },
      data: {
        x: [],
        y: []
      },
      backgroundIcon: {
        svgIcon: 'memory',
        margin: {}
      },
      themeColor: themeColor,
      icon: icon,
      cardState: CardStates.LOADING
    };
  }

  public static clusterLevelDiskHeroCard(themeColor: string, icon: Icon): HeroCardOptions {
    return {
      title: 'Storage',
      firstStat: {
        value: null,
        description: 'Disk Usage',
        tooltip: 'Averaged over past 5m'
      },
      secondStat: {
        value: null,
        description: '24h Avg',
        tooltip: 'Averaged over past 24h'
      },
      thirdStat: {
        value: null,
        description: 'Highest Disk Usage',
        tooltip: 'Averaged over past 5m'
      },
      data: {
        x: [],
        y: []
      },
      backgroundIcon: {
        svgIcon: 'storage',
        margin: {}
      },
      themeColor: themeColor,
      icon: icon,
      cardState: CardStates.LOADING
    };
  }

  public static detailLevelCPUHeroCard(themeColor: string, icon: Icon): HeroCardOptions {
    return {
      title: 'CPU',
      firstStat: {
        value: null,
        description: 'CPU Usage',
        tooltip: 'Averaged over past 5m'
      },
      secondStat: {
        value: null,
        description: '24h Avg',
        tooltip: 'Averaged over past 24h'
      },
      thirdStat: {
        value: null,
        description: 'Cluster Usage',
        tooltip: 'Averaged over past 5m'
      },
      data: {
        x: [],
        y: []
      },
      backgroundIcon: {
        svgIcon: 'cpu',
        margin: {}
      },
      themeColor: themeColor,
      icon: icon
    };
  }

  public static detailLevelMemHeroCard(themeColor: string, icon: Icon): HeroCardOptions {
    return {
      title: 'Memory',
      firstStat: {
        value: null,
        description: 'Memory Usage',
        tooltip: 'Averaged over past 5m'
      },
      secondStat: {
        value: null,
        description: '24h Avg',
        tooltip: 'Averaged over past 24h'
      },
      thirdStat: {
        value: null,
        description: 'Cluster Usage',
        tooltip: 'Averaged over past 5m'
      },
      data: {
        x: [],
        y: []
      },
      backgroundIcon: {
        svgIcon: 'memory',
        margin: {}
      },
      themeColor: themeColor,
      icon: icon
    };
  }

  public static detailLevelDiskHeroCard(themeColor: string, icon: Icon): HeroCardOptions {
    return {
      title: 'Storage',
      firstStat: {
        value: null,
        description: 'Disk Usage',
        tooltip: 'Averaged over past 5m'
      },
      secondStat: {
        value: null,
        description: '24h Avg',
        tooltip: 'Averaged over past 24h'
      },
      thirdStat: {
        value: null,
        description: 'Cluster Usage',
        tooltip: 'Averaged over past 5m'
      },
      data: {
        x: [],
        y: []
      },
      backgroundIcon: {
        svgIcon: 'storage',
        margin: {}
      },
      themeColor: themeColor,
      icon: icon
    };
  }


}
