import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { MatDialog } from '@angular/material';
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
import { Subscription } from 'rxjs';
import { StatArrowDirection, CardStates } from '@app/components/shared/basecard/basecard.component';
import {Animations} from '@app/animations';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterupdateComponent } from './clusterupdate/clusterupdate.component';
import { NodeConditionValues } from '@app/components/cluster-group/naples';

@Component({
  selector: 'app-cluster',
  encapsulation: ViewEncapsulation.None,
  templateUrl: './cluster.component.html',
  animations: [Animations],
  styleUrls: ['./cluster.component.scss']
})
export class ClusterComponent extends BaseComponent implements OnInit, OnDestroy {
  certMode = false;
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

  lastUpdateTime: string = '';

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
  dialogRef: any;
  telemetryKind: string = 'Node';

  constructor(
    private _clusterService: ClusterService,
    protected uiconfigService: UIConfigsService,
    protected _controllerService: ControllerService,
    protected metricsqueryService: MetricsqueryService,
    protected dialog: MatDialog,
  ) {
    super(_controllerService, uiconfigService);
  }

  ngOnInit() {
    this.getCluster();
    this.getNodes();
    this.getMetrics();

    this._controllerService.setToolbarData({
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
      this._controllerService.webSocketErrorHandler('Failed to get Cluster'),
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
      this._controllerService.webSocketErrorHandler('Failed to get Nodes'),
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
          this.lastUpdateTime = new Date().toISOString();
          this.tryGenCharts();
        }
      },
      (err) => {
        MetricsUtility.setCardStatesFailed(this.heroCards);
      }
    );
    this.subscriptions.push(sub);
  }



  showUpdateCluster() {
    this.dialogRef = this.dialog.open(ClusterupdateComponent, {
      width: '800px',
      height: '500px',
      hasBackdrop : true,
      data : {
        nodes : this.nodes,
        cluster : this.cluster
      }
    });
  }


  timeSeriesQuery(): MetricsPollingQuery {
    return MetricsUtility.timeSeriesQueryPolling(this.telemetryKind, ['CPUUsedPercent', 'MemUsedPercent', 'DiskUsedPercent']);
  }

  avgQuery(): MetricsPollingQuery {
    return MetricsUtility.pastFiveMinAverageQueryPolling(this.telemetryKind);
  }

  displayCondition(node: ClusterNode) {
    return Utility.getNodeCondition(node);
  }

  avgDayQuery(): MetricsPollingQuery {
    return MetricsUtility.pastDayAverageQueryPolling(this.telemetryKind);
  }

  maxNodeQuery(): MetricsPollingQuery {
    return MetricsUtility.maxObjQueryPolling(this.telemetryKind);
  }

  private tryGenCharts() {
    // Only require avg 5 min data and avg day data
    // before we show the cards
    if (MetricsUtility.resultHasData(this.avgData) &&
      MetricsUtility.resultHasData(this.avgDayData)) {
      this.genCharts('CPUUsedPercent', this.cpuChartData);
      this.genCharts('MemUsedPercent', this.memChartData);
      this.genCharts('DiskUsedPercent', this.diskChartData);
    } else {
      MetricsUtility.setCardStatesNoData(this.heroCards);
    }
  }

  /**
   * An example of the format of the returned data is in MetricsUtility.ts
   */
  private genCharts(fieldName: string, heroCard: HeroCardOptions) {
    // Time series graph
    if (MetricsUtility.resultHasData(this.timeSeriesData)) {
      const timeSeriesData = this.timeSeriesData;

      const data = MetricsUtility.transformToChartjsTimeSeries(timeSeriesData.series[0], fieldName);
      heroCard.lineData.data = data;
    }

    // current avg
    if (MetricsUtility.resultHasData(this.avgData)) {
      const index = MetricsUtility.findFieldIndex(this.avgData.series[0].columns, fieldName);
      heroCard.firstStat.numericValue = Math.round(this.avgData.series[0].values[0][index]);
      heroCard.firstStat.value = Math.round(this.avgData.series[0].values[0][index]) + '%';
    }

    // Avg day
    const avgDayData = this.avgDayData;
    if (avgDayData.series[0].values.length !== 0) {
      const index = MetricsUtility.findFieldIndex(this.avgDayData.series[0].columns, fieldName);
      heroCard.secondStat.numericValue = Math.round(avgDayData.series[0].values[0][index]);
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
        const thirdStatName = maxNode.name;
        let thirdStat = thirdStatName;

        // VS-736
        thirdStat = Utility.getHeroCardDisplayValue(thirdStat);
        Utility.customizeHeroCardThirdStat(thirdStat, heroCard, thirdStatName);

        thirdStat += ' (' + Math.round(maxNode.max) + '%)';
        heroCard.thirdStat.numericValue = Math.round(maxNode.max);
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

  getNodeStatusClass(node: ClusterNode): string {
    return (this.displayCondition(node) === NodeConditionValues.HEALTHY) ? 'cluster-node-status-healthy' : 'cluster-node-status-unhealthy';
  }
}
