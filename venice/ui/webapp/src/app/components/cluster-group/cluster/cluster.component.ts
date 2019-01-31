import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { HeroCardOptions } from '@app/components/shared/herocard/herocard.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MetricsPollingOptions, MetricsqueryService, TelemetryPollingMetricQueries, MetricsPollingQuery } from '@app/services/metricsquery.service';
import { ClusterCluster, ClusterNode } from '@sdk/v1/models/generated/cluster';
import { Telemetry_queryMetricsQuerySpec } from '@sdk/v1/models/generated/telemetry_query';
import { ITelemetry_queryMetricsQueryResponse, ITelemetry_queryMetricsQueryResult } from '@sdk/v1/models/telemetry_query';
import { MessageService } from 'primeng/primeng';
import { Subscription } from 'rxjs';
import { StatArrowDirection, CardStates } from '@app/components/shared/basecard/basecard.component';

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

  timeSeriesData: ITelemetry_queryMetricsQueryResult;
  avgData: ITelemetry_queryMetricsQueryResult;
  avgDayData: ITelemetry_queryMetricsQueryResult;
  maxObjData: ITelemetry_queryMetricsQueryResult;

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
        this.clusterEventUtility.processEvents(response);
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
        this.nodeEventUtility.processEvents(response);
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
    const queryList: TelemetryPollingMetricQueries = {
      queries: [],
      tenant: Utility.getInstance().getTenant()
    };
    queryList.queries.push(this.timeSeriesQuery());
    queryList.queries.push(this.avgQuery());
    queryList.queries.push(this.avgDayQuery());
    queryList.queries.push(this.maxNodeQuery());
    const sub = this.metricsqueryService.pollMetrics('clusterCards', queryList).subscribe(
      (data: ITelemetry_queryMetricsQueryResponse) => {
        if (data && data.results && data.results.length === 4) {
          this.timeSeriesData = data.results[0];
          this.avgData = data.results[1];
          this.avgDayData = data.results[2];
          this.maxObjData = data.results[3];
          this.tryGenCharts();
        }
      },
      (err) => {
        this.setChartErrorStates();
      }
    );
    this.subscriptions.push(sub);
  }

  timeSeriesQuery(): MetricsPollingQuery {
    const query: Telemetry_queryMetricsQuerySpec = MetricsUtility.timeSeriesQuery(this.telemetryKind);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.timeSeriesQueryUpdate,
      mergeFunction: MetricsUtility.timeSeriesQueryMerge
    };
    return { query: query, pollingOptions: pollOptions };
  }

  avgQuery(): MetricsPollingQuery {
    const query: Telemetry_queryMetricsQuerySpec = MetricsUtility.pastFiveMinAverageQuery(this.telemetryKind);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.pastFiveMinQueryUpdate,
    };

    return { query: query, pollingOptions: pollOptions };
  }

  avgDayQuery(): MetricsPollingQuery {
    const query: Telemetry_queryMetricsQuerySpec = MetricsUtility.pastDayAverageQuery(this.telemetryKind);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.pastDayAverageQueryUpdate,
    };

    return { query: query, pollingOptions: pollOptions };
  }

  maxNodeQuery(): MetricsPollingQuery {
    const query: Telemetry_queryMetricsQuerySpec = MetricsUtility.maxObjQuery(this.telemetryKind);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.maxObjQueryUpdate,
      mergeFunction: MetricsUtility.maxObjQueryMerge
    };

    return { query: query, pollingOptions: pollOptions };
  }

  private tryGenCharts() {
    // Only require avg 5 min data and avg day data
    // before we show the cards
    if (MetricsUtility.resultHasData(this.avgData) &&
      MetricsUtility.resultHasData(this.avgDayData)) {
      this.genCharts('mean_CPUUsedPercent', this.cpuChartData);
      this.genCharts('mean_MemUsedPercent', this.memChartData);
      this.genCharts('mean_DiskUsedPercent', this.diskChartData);
    } else {
      this.setChartErrorStates();
    }
  }

  private setChartErrorStates() {
    this.heroCards.forEach((card) => {
      card.cardState = CardStates.FAILED;
    });
  }

  /**
   * An example of the format of the returned data is in MetricsUtility.ts
   */
  private genCharts(fieldName: string, heroCard: HeroCardOptions) {
    // Time series graph
    if (MetricsUtility.resultHasData(this.timeSeriesData)) {
      const timeSeriesData = this.timeSeriesData;

      const index = timeSeriesData.series[0].columns.indexOf(fieldName);
      const data = Utility.transformToPlotly(timeSeriesData.series[0].values, 0, index);
      heroCard.data = data;
    }

    // current avg
    if (MetricsUtility.resultHasData(this.avgData)) {
      const index = this.avgData.series[0].columns.indexOf(fieldName);
      heroCard.firstStat.value = Math.round(this.avgData.series[0].values[0][index]) + '%';
    }

    // Avg day
    const avgDayData = this.avgDayData;
    if (avgDayData.series[0].values.length !== 0) {
      const index = this.avgDayData.series[0].columns.indexOf(fieldName);
      heroCard.secondStat.value = Math.round(avgDayData.series[0].values[0][index]) + '%';
    }

    // For determining arrow direction, we compare the current value to the average value
    if (heroCard.firstStat == null || heroCard.firstStat.value == null || heroCard.secondStat == null || heroCard.secondStat.value == null) {
      heroCard.arrowDirection = StatArrowDirection.HIDDEN;
    } else if (heroCard.firstStat.value > heroCard.secondStat.value) {
      heroCard.arrowDirection = StatArrowDirection.UP;
    } else if (heroCard.firstStat.value < heroCard.secondStat.value) {
      heroCard.arrowDirection = StatArrowDirection.DOWN;
    } else {
      heroCard.arrowDirection = StatArrowDirection.HIDDEN;
    }

    // Max Node
    if (MetricsUtility.resultHasData(this.maxObjData)) {
      const maxNode = MetricsUtility.maxObjUtility(this.maxObjData, fieldName);
      if (maxNode == null || maxNode.max === -1) {
        heroCard.thirdStat.value = null;
      } else {
        // Removing Node- prefix from the name and adding value
        const thirdStatName = maxNode.name.substring(this.telemetryKind.length + 1);
        let thirdStat = thirdStatName;
        if (thirdStat.length > 10) {
          thirdStat = thirdStat.substring(0, 11) + '...';
        }
        thirdStat += ' (' + Math.round(maxNode.max) + '%)';
        heroCard.thirdStat.value = thirdStat;
        heroCard.thirdStat.url = '/cluster/cluster/' + thirdStatName;
      }
    }

    if (heroCard.cardState !== CardStates.READY) {
      heroCard.cardState = CardStates.READY;
    }
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }
}
