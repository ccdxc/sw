import { Component, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { AlertseventsComponent } from '@app/components/shared/alertsevents/alertsevents.component';
import { HeroCardOptions, StatArrowDirection } from '@app/components/shared/herocard/herocard.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MetricsqueryService, MetricsPollingOptions } from '@app/services/metricsquery.service';
import { ClusterCluster, ClusterNode } from '@sdk/v1/models/generated/cluster';
import { IMetrics_queryQuerySpec, Metrics_queryQuerySpec, Metrics_queryQuerySpec_function } from '@sdk/v1/models/generated/metrics_query';
import { IMetrics_queryQueryResponse } from '@sdk/v1/models/metrics_query';
import { Table } from 'primeng/table';
import { Subscription } from 'rxjs/Subscription';
import { MessageService } from 'primeng/primeng';

@Component({
  selector: 'app-cluster',
  encapsulation: ViewEncapsulation.None,
  templateUrl: './cluster.component.html',
  styleUrls: ['./cluster.component.scss']
})
export class ClusterComponent extends BaseComponent implements OnInit, OnDestroy {
  @ViewChild('nodestable') nodesTable: Table;
  @ViewChild(AlertseventsComponent) alertsEventsComponent: AlertseventsComponent;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/cluster/ico-cluster-black.svg'
  };
  cluster: ClusterCluster = new ClusterCluster();
  // Used for processing the stream events
  clusterEventUtility: HttpEventUtility<ClusterCluster>;
  nodeEventUtility: HttpEventUtility<ClusterNode>;

  clusterArray: ReadonlyArray<ClusterCluster> = [];
  nodes: ReadonlyArray<ClusterNode> = [];
  selectedNode: ClusterNode = null;

  phaseEnum = new ClusterNode().status.getPropInfo('phase').enum;

  chartVars = ['cpuChartData', 'memChartData', 'diskChartData'];
  clusterCardColor = '#b592e3';

  clusterIcon: Icon = {
    margin: {
      top: '10px',
      left: '10px'
    },
    svgIcon: 'cluster'
  };

  cpuChartData: HeroCardOptions = {
    title: 'CPU',
    firstStat: {
      value: '65%',
      description: 'CPU Usage'
    },
    secondStat: {
      value: 72.3,
      description: 'Average'
    },
    thirdStat: {
      value: 'Node 12345',
      description: 'Highest CPU Usage'
    },
    data: {
      x: [],
      y: []
    },
    backgroundIcon: {
      svgIcon: 'cpu',
      margin: {}
    },
    themeColor: this.clusterCardColor,
    icon: this.clusterIcon
  };

  memChartData: HeroCardOptions = {
    title: 'Memory',
    firstStat: {
      value: '65%',
      description: 'Memory Usage'
    },
    secondStat: {
      value: 72.3,
      description: 'Avg'
    },
    thirdStat: {
      value: 'Node 12345',
      description: 'Highest Memory Usage'
    },
    data: {
      x: [],
      y: []
    },
    backgroundIcon: {
      svgIcon: 'memory',
      margin: {}
    },
    themeColor: this.clusterCardColor,
    icon: this.clusterIcon
  };

  diskChartData: HeroCardOptions = {
    title: 'Storage',
    firstStat: {
      value: '65%',
      description: 'Disk Usage'
    },
    secondStat: {
      value: 72.3,
      description: 'Avg'
    },
    thirdStat: {
      value: 'Node 12345',
      description: 'Highest Disk Usage'
    },
    data: {
      x: [],
      y: []
    },
    backgroundIcon: {
      svgIcon: 'storage',
      margin: {}
    },
    themeColor: this.clusterCardColor,
    icon: this.clusterIcon
  };
  heroCards = [
    this.cpuChartData,
    this.memChartData,
    this.diskChartData
  ];

  subscriptions: Subscription[] = [];

  clusterTimeSeriesData: IMetrics_queryQueryResponse;
  clusterAvgData: IMetrics_queryQueryResponse;
  clusterMaxNodeData: IMetrics_queryQueryResponse;

  constructor(
    private _clusterService: ClusterService,
    protected _controllerService: ControllerService,
    protected messageService: MessageService,
    protected metricsqueryService: MetricsqueryService,
  ) {
    super(_controllerService, messageService);
  }

  ngOnInit() {
    this.getCluster();
    this.getNodes();
    this.getMetrics();

    this._controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [{ label: 'Cluster', url: '' }, { label: 'Cluster', url: '' }]
    });
  }

  getCluster() {
    this.clusterEventUtility = new HttpEventUtility<ClusterCluster>(ClusterCluster, true);
    this.clusterArray = this.clusterEventUtility.array as ReadonlyArray<ClusterCluster>;
    const subscription = this._clusterService.WatchCluster().subscribe(
      response => {
        const body: any = response.body;
        this.clusterEventUtility.processEvents(body);
        if (this.clusterArray.length > 0) {
          this.cluster = this.clusterArray[0];
        }
      },
      this.restErrorHandler('Failed to get Cluster info')
    );
    this.subscriptions.push(subscription);
  }

  getNodes() {
    this.nodeEventUtility = new HttpEventUtility<ClusterNode>(ClusterNode);
    this.nodes = this.nodeEventUtility.array;
    const subscription = this._clusterService.WatchNode().subscribe(
      response => {
        const body: any = response.body;
        this.nodeEventUtility.processEvents(body);
      },
      this.restErrorHandler('Failed to get Node info')
    );
    this.subscriptions.push(subscription);
  }

  /**
   * We start 3 metric polls.
   * Time series poll
   *  - Fetches the past 24 hours, averaging in
   *    5 min buckets (starting at the hour) across all nodes
   *  - Used for line graph as well as the current value stat
   * AvgPoll
   *   - Fetches the average over the past 24 hours across all nodes
   * MaxNode poll
   *   - Fetches the average over the past 5 min bucket. Grouped by node
   *     - Rounds down to the nearest 5 min increment.
   *       Ex. 11:52 -> fetches data from 11:50 till current time
   *
   * When you query for 5 min intervals, they are automatically aligned by the hour
   * We always round down to the last completed 5 min interval.
   *
   * Cluster level display
   *  - Time series graph of all the nodes averaged together, avg into 5 min buckets
   *  - Current avg of last 5 min
   *  - Average of past day
   *  - Node using the most in the last 5 min
   */
  getMetrics() {
    this.timeSeriesQuery();
    this.avgQuery();
    this.maxNodeQuery();
  }

  timeSeriesQuery() {
    const timeSeriesQuery: IMetrics_queryQuerySpec = {
      'kind': 'Node',
      'meta': {
        'tenant': Utility.getInstance().getTenant(),
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

    const query = new Metrics_queryQuerySpec(timeSeriesQuery);

    // Since we are averaging over 5 min buckets, we always query from the last 5 min window increment
    const timeSeriesUpdate = (queryBody) => {
      queryBody['start-time'] = queryBody['end-time'];
      queryBody['end-time'] = Utility.roundDownTime(5).toISOString() as any;
    };

    // Drops any values that are older than 24 hours from the current time.
    // We then add on the newer values.
    const timeSeriesMerge = (currValue, newData) => {
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
      if (this.hasData(newData)) {
        const data = newData.results[0].series[0].values;
        // Push on the new data
        currValue.results[0].series[0].values.push(...data);
      }
      return currValue;
    };

    const pollOptions: MetricsPollingOptions = {
      timeUpdater: timeSeriesUpdate,
      mergeFunction: timeSeriesMerge
    };

    const sub = this.metricsqueryService.pollMetrics('clusterTimeSeriesData', query, pollOptions).subscribe(
      (data) => {
        this.clusterTimeSeriesData = data;
        this.genClusterCharts();
      }
    );
    this.subscriptions.push(sub);
  }

  avgQuery() {
    const avgQuery: IMetrics_queryQuerySpec = {
      'kind': 'Node',
      'meta': {
        'tenant': Utility.getInstance().getTenant()
      },
      function: Metrics_queryQuerySpec_function.MEAN,
      // We don't specify the fields we need, as specifying more than one field
      // while using the average function isn't supported by the backend.
      // Instead we leave blank and get all fields
      fields: [],
      'start-time': 'now() - 24h' as any,
      'end-time': 'now()' as any,
    };

    const query = new Metrics_queryQuerySpec(avgQuery);
    const avgTimeUpdate = (queryBody: IMetrics_queryQuerySpec) => {
      queryBody['start-time'] = 'now() - 24h' as any;
      queryBody['end-time'] = 'now()' as any;
    };

    const pollOptions = {
      timeUpdater: avgTimeUpdate,
    };

    const sub = this.metricsqueryService.pollMetrics('clusterAvgData', query, pollOptions).subscribe(
      (data) => {
        this.clusterAvgData = data;
        this.genClusterCharts();
      }
    );
    this.subscriptions.push(sub);
  }

  maxNodeQuery() {
    const maxNodeQuery: IMetrics_queryQuerySpec = {
      'kind': 'Node',
      'meta': {
        'tenant': Utility.getInstance().getTenant()
      },
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

    const query = new Metrics_queryQuerySpec(maxNodeQuery);
    const maxNodeTimeUpdate = (queryBody: IMetrics_queryQuerySpec) => {
      queryBody['start-time'] = new Date(Utility.roundDownTime(5).getTime() - 1000 * 50 * 5).toISOString() as any,
        queryBody['end-time'] = Utility.roundDownTime(5).toISOString() as any;
    };

    const maxNodeMerge = (currData, newData) => {
      // since we round down to get the last 5 min bucket, there's a chance
      // that we can back null data, since no new metrics have been reported.
      // Data should have been filtered in metricsquery services's processData
      if (!this.hasData(newData)) {
        // no new data, keep old value
        return currData;
      }
      return newData;
    };

    const pollOptions = {
      timeUpdater: maxNodeTimeUpdate,
      mergeFunction: maxNodeMerge
    };

    const sub = this.metricsqueryService.pollMetrics('clusterMaxNodeData', query, pollOptions).subscribe(
      (data) => {
        this.clusterMaxNodeData = data;
        this.genClusterCharts();
      }
    );
    this.subscriptions.push(sub);
  }

  onNodeClick(node: ClusterNode) {
    if (!node) {
      return;
    }
    if (this.selectedNode === node) {
      this.selectedNode = null;
    } else {
      this.selectedNode = node;
    }
  }

  hasData(resp: IMetrics_queryQueryResponse) {
    if (resp && resp.results.length !== 0 && resp.results[0].series.length !== 0 && resp.results[0].series[0].values.length !== 0) {
      return true;
    } else {
      console.log(resp);
      return false;
    }
  }

  private genClusterCharts() {
    if (this.hasData(this.clusterTimeSeriesData) &&
      this.hasData(this.clusterAvgData) &&
      this.hasData(this.clusterMaxNodeData)) {
      this.genClusterChart('mean_CPUUsedPercent', this.cpuChartData);
      this.genClusterChart('mean_MemUsedPercent', this.memChartData);
      this.genClusterChart('mean_DiskUsedPercent', this.diskChartData);
    }
  }

  /**
   * serverData is like:
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
   *  grouped by node1/node2 ( it is column['Name'])
   */
  private genClusterChart(fieldName, heroCard: HeroCardOptions) {
    const timeSeriesData = this.clusterTimeSeriesData;

    const index = timeSeriesData.results[0].series[0].columns.indexOf(fieldName);
    const data = Utility.transformToPlotly(timeSeriesData.results[0].series[0].values, 0, index);
    heroCard.data = data;

    // Current stat calculation - we take the last point
    heroCard.firstStat.value = Math.round(data.y[data.y.length - 1]) + '%';

    // Avg
    const avgData = this.clusterAvgData;
    if (avgData.results[0].series[0].values.length !== 0) {
      heroCard.secondStat.value = Math.round(avgData.results[0].series[0].values[0][index]) + '%';
    }

    // For determining arrow direction, we compare the current value to the average value
    if (heroCard.firstStat.value > heroCard.secondStat.value) {
      heroCard.arrowDirection = StatArrowDirection.UP;
    } else if (heroCard.firstStat.value < heroCard.secondStat.value) {
      heroCard.arrowDirection = StatArrowDirection.DOWN;
    } else {
      heroCard.arrowDirection = StatArrowDirection.HIDDEN;
    }

    // Max Node
    const maxNode = this.maxNodeUtility(this.clusterMaxNodeData, fieldName);
    // Removing Node- prefix from the name and adding value
    if (maxNode.max === -1) {
      heroCard.thirdStat.value = null;
    } else {
      const thirdStat = maxNode.name.substring(5) + ' - ' + Math.round(maxNode.max) + '%';
      heroCard.thirdStat.value = thirdStat;
    }

    if (!heroCard.isReady) {
      heroCard.isReady = true;
    }
  }

  /**
   * Returns the name and avg of the node with the highest avg
   * @param data
   */
  maxNodeUtility(data, fieldName): { name: string, max: number } {
    const index = this.clusterAvgData.results[0].series[0].columns.indexOf(fieldName);
    const nodeMaxList = [];
    data.results.forEach((res) => {
      const series = res.series;
      const key = series[0].tags.reporterID;
      let max;
      if (series[0].values.length !== 0) {
        max = series[0].values[0][index];
      } else {
        max = -1;
      }
      nodeMaxList.push({ name: key, max: max });
    });
    const maxNode = Utility.getLodash().maxBy(nodeMaxList, 'max');
    return maxNode;
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }
}
