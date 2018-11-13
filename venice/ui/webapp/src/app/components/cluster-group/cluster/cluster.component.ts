import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { HeroCardOptions, StatArrowDirection } from '@app/components/shared/herocard/herocard.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MetricsPollingOptions, MetricsqueryService } from '@app/services/metricsquery.service';
import { ClusterCluster, ClusterNode } from '@sdk/v1/models/generated/cluster';
import { Metrics_queryQuerySpec } from '@sdk/v1/models/generated/metrics_query';
import { IMetrics_queryQueryResponse } from '@sdk/v1/models/metrics_query';
import { MessageService } from 'primeng/primeng';
import { Subscription } from 'rxjs';

@Component({
  selector: 'app-cluster',
  encapsulation: ViewEncapsulation.None,
  templateUrl: './cluster.component.html',
  styleUrls: ['./cluster.component.scss']
})
export class ClusterComponent extends BaseComponent implements OnInit, OnDestroy {

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
  selectedObj: ClusterNode = null;

  phaseEnum = new ClusterNode().status.getPropInfo('phase').enum;

  cardColor = '#b592e3';

  cardIcon: Icon = {
    margin: {
      top: '10px',
      left: '10px'
    },
    svgIcon: 'cluster'
  };

  cpuChartData: HeroCardOptions = MetricsUtility.clusterLevelCPUHeroCard(this.cardColor, this.cardIcon);

  memChartData: HeroCardOptions = MetricsUtility.clusterLevelMemHeroCard(this.cardColor, this.cardIcon);

  diskChartData: HeroCardOptions = MetricsUtility.clusterLevelDiskHeroCard(this.cardColor, this.cardIcon);

  heroCards = [
    this.cpuChartData,
    this.memChartData,
    this.diskChartData
  ];

  subscriptions: Subscription[] = [];

  timeSeriesData: IMetrics_queryQueryResponse;
  avgData: IMetrics_queryQueryResponse;
  maxObjData: IMetrics_queryQueryResponse;

  telemetryKind: string = 'Node';

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
      breadcrumb: [{ label: 'Cluster', url: Utility.getBaseUIUrl() + 'cluster/cluster' }]
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
    const timeSeriesQuery: Metrics_queryQuerySpec = MetricsUtility.timeSeriesQuery(this.telemetryKind);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.timeSeriesQueryUpdate,
      mergeFunction: MetricsUtility.timeSeriesQueryMerge
    };

    const sub = this.metricsqueryService.pollMetrics('clusterTimeSeriesData', timeSeriesQuery, pollOptions).subscribe(
      (data) => {
        this.timeSeriesData = data;
        this.tryGenCharts();
      }
    );
    this.subscriptions.push(sub);
  }

  avgQuery() {
    const avgQuery: Metrics_queryQuerySpec = MetricsUtility.pastDayAverageQuery(this.telemetryKind);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.pastDayAverageQueryUpdate,
    };

    const sub = this.metricsqueryService.pollMetrics('clusterAvgData', avgQuery, pollOptions).subscribe(
      (data) => {
        this.avgData = data;
        this.tryGenCharts();
      }
    );
    this.subscriptions.push(sub);
  }

  maxNodeQuery() {
    const query: Metrics_queryQuerySpec = MetricsUtility.maxObjQuery(this.telemetryKind);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.maxObjQueryUpdate,
      mergeFunction: MetricsUtility.maxObjQueryMerge
    };

    const sub = this.metricsqueryService.pollMetrics('clusterMaxNodeData', query, pollOptions).subscribe(
      (data) => {
        this.maxObjData = data;
        this.tryGenCharts();
      }
    );
    this.subscriptions.push(sub);
  }

  private tryGenCharts() {
    if (MetricsUtility.hasData(this.timeSeriesData) &&
      MetricsUtility.hasData(this.avgData) &&
      MetricsUtility.hasData(this.maxObjData)) {
      this.genClusterChart('mean_CPUUsedPercent', this.cpuChartData);
      this.genClusterChart('mean_MemUsedPercent', this.memChartData);
      this.genClusterChart('mean_DiskUsedPercent', this.diskChartData);
    }
  }

  /**
   * An example of the format of the returned data is in MetricsUtility.ts
   */
  private genClusterChart(fieldName: string, heroCard: HeroCardOptions) {
    const timeSeriesData = this.timeSeriesData;

    const index = timeSeriesData.results[0].series[0].columns.indexOf(fieldName);
    const data = Utility.transformToPlotly(timeSeriesData.results[0].series[0].values, 0, index);
    heroCard.data = data;

    // Current stat calculation - we take the last point
    heroCard.firstStat.value = Math.round(data.y[data.y.length - 1]) + '%';

    // Avg
    const avgData = this.avgData;
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
    const maxNode = MetricsUtility.maxObjUtility(this.maxObjData, fieldName);
    if (maxNode == null || maxNode.max === -1) {
      heroCard.thirdStat.value = null;
    } else {
      // Removing Node- prefix from the name and adding value
      const thirdStatName = maxNode.name.substring(this.telemetryKind.length + 1)
      let thirdStat = thirdStatName;
      if (thirdStat.length > 10) {
        thirdStat = thirdStat.substring(0, 11) + '...';
      }
      thirdStat += ' (' + Math.round(maxNode.max) + '%)';
      heroCard.thirdStat.value = thirdStat;
      heroCard.thirdStat.url = '/cluster/cluster/' + thirdStatName;
    }

    if (!heroCard.isReady) {
      heroCard.isReady = true;
    }
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }
}
