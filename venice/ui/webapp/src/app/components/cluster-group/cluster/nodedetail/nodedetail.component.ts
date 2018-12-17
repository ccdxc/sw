import { Component, OnInit, OnDestroy } from '@angular/core';
import { BaseComponent } from '@app/components/base/base.component';
import { ControllerService } from '@app/services/controller.service';
import { ActivatedRoute } from '@angular/router';
import { MessageService } from 'primeng/primeng';
import { ClusterNode } from '@sdk/v1/models/generated/cluster';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Utility } from '@app/common/Utility';
import { ClusterService } from '@app/services/generated/cluster.service';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { HeroCardOptions, StatArrowDirection, CardStates } from '@app/components/shared/herocard/herocard.component';
import { IMetrics_queryQueryResponse, IMetrics_queryQueryResult } from '@sdk/v1/models/metrics_query';
import { Metrics_queryQuerySpec, IMetrics_queryQuerySpec, Metrics_queryQuerySpec_function } from '@sdk/v1/models/generated/metrics_query';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { MetricsPollingOptions, MetricsqueryService, MetricsPollingQueries, MetricsPollingQuery } from '@app/services/metricsquery.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { AlertsEventsSelector } from '@app/components/shared/alertsevents/alertsevents.component';
import { map } from 'rxjs/internal/operators';

/**
 * If a user navigates to a node that doesn't exist
 * Ex. /cluster/cluster/fakeNode
 * they will be shown a node does not exist overlay
 * If the node becomes created while they are on the page
 * it will immediately go away and show the node
 * If a node is deleted while a user is looking at it,
 * it will immediately show a node is deleted overlay.
 * Again, if the node becomes recreated the overlay will disappear
 */
@Component({
  selector: 'app-nodedetail',
  templateUrl: './nodedetail.component.html',
  styleUrls: ['./nodedetail.component.scss']
})
export class NodedetailComponent extends BaseComponent implements OnInit, OnDestroy {
  subscriptions = [];

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/cluster/nodes/ico-node-black.svg'
  };

  cardColor = '#61b3a0';
  cardIcon: Icon = {
    margin: {
      top: '10px',
      left: '10px'
    },
    svgIcon: 'node'
  };

  // Id of the object the user has navigated to
  selectedId: string;
  selectedObj: ClusterNode;

  // Holds all objects, should be only one item in the array
  objList: ReadonlyArray<ClusterNode>;
  objEventUtility: HttpEventUtility<ClusterNode>;

  // Whether we show a deletion overlay
  showDeletionScreen: boolean;

  // Whether we show a missing overlay
  showMissingScreen: boolean;

  cpuChartData: HeroCardOptions = MetricsUtility.detailLevelCPUHeroCard(this.cardColor, this.cardIcon);

  memChartData: HeroCardOptions = MetricsUtility.detailLevelMemHeroCard(this.cardColor, this.cardIcon);

  diskChartData: HeroCardOptions = MetricsUtility.detailLevelDiskHeroCard(this.cardColor, this.cardIcon);

  heroCards = [
    this.cpuChartData,
    this.memChartData,
    this.diskChartData
  ];

  timeSeriesData: IMetrics_queryQueryResult;
  avgData: IMetrics_queryQueryResult;
  avgDayData: IMetrics_queryQueryResult;
  clusterAvgData: IMetrics_queryQueryResult;

  telemetryKind: string = 'Node';

  alertseventsSelector: AlertsEventsSelector = {
    eventSelector: 'source.node-name=' + this.selectedId,
    alertSelector: 'status.source.node-name=' + this.selectedId
  };

  constructor(protected _controllerService: ControllerService,
    private _route: ActivatedRoute,
    protected clusterService: ClusterService,
    protected metricsqueryService: MetricsqueryService,
    protected uiconfigService: UIConfigsService,
    protected messageService: MessageService
  ) {
    super(_controllerService, messageService, uiconfigService);
  }

  ngOnInit() {
    this.initializeData();
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'NodedetailComponent', 'state': Eventtypes.COMPONENT_INIT });
    this._route.paramMap.subscribe(params => {
      const id = params.get('id');
      this.selectedId = id;
      this.initializeData();
      this.getNodedetails();
      this._controllerService.setToolbarData({
        buttons: [],
        breadcrumb: [
          { label: 'Cluster', url: Utility.getBaseUIUrl() + 'cluster/cluster' },
          { label: id, url: Utility.getBaseUIUrl() + 'cluster/cluster/' + id }]
      });
    });
  }

  initializeData() {
    // Initializing variables so that state is cleared between routing of different
    // sgpolicies
    // Ex. /cluster/node1-> /cluster/node2
    this.subscriptions.forEach(sub => {
      sub.unsubscribe();
    });
    this.subscriptions = [];
    this.objList = [];
    this.showDeletionScreen = false;
    this.showMissingScreen = false;
    this.selectedObj = null;
    this.timeSeriesData = null;
    this.avgData = null;
    this.avgDayData = null;
    this.clusterAvgData = null;
    this.objList = [];
    this.selectedObj = null;
    this.alertseventsSelector = null;

    this.heroCards.forEach((card) => {
      card.firstStat.value = null;
      card.secondStat.value = null;
      card.thirdStat.value = null;
      card.data = { x: [], y: [] };
      card.cardState = CardStates.LOADING;
    });
  }

  getNodedetails() {
    // We perform a get as well as a watch so that we can know if the object the user is
    // looking for exists or not.
    const getSubscription = this.clusterService.GetNode(this.selectedId).subscribe(
      response => {
        // We do nothing, and wait for the callback of the watch to populate the view
      },
      error => {
        // If we receive any error code we display object is missing
        // TODO: Update to be more descriptive based on error message
        this.showMissingScreen = true;
        this.heroCards.forEach(card => {
          card.cardState = CardStates.READY;
        });
      }
    );
    this.objEventUtility = new HttpEventUtility<ClusterNode>(ClusterNode);
    this.objList = this.objEventUtility.array;
    const subscription = this.clusterService.WatchNode({ 'field-selector': 'meta.name=' + this.selectedId }).subscribe(
      response => {
        this.objEventUtility.processEvents(response);
        if (this.objList.length > 1) {
          // because of the name selector, we should
          // have only got one object
          console.error(
            'Received more than one node. Expected '
            + this.selectedId + ', received ' +
            this.objList.map((node) => node.meta.name).join(', '));
        }
        if (this.objList.length > 0) {
          // In case object was deleted and then readded while we are on the same screen
          this.showDeletionScreen = false;
          // In case object wasn't created yet and then was added while we are on the same screen
          this.showMissingScreen = false;
          this.selectedObj = this.objList[0];
          this.startMetricPolls();
          this.alertseventsSelector = {
            eventSelector: 'source.node-name=' + this.selectedId,
            alertSelector: 'status.source.node-name=' + this.selectedId,
          };
        } else {
          // Must have received a delete event.
          this.showDeletionScreen = true;
          this.heroCards.forEach(card => {
            card.cardState = CardStates.READY;
          });
          this.selectedObj = null;
        }
      },
      this.restErrorHandler('Failed to get Node ' + this.selectedId)
    );
    this.subscriptions.push(subscription);
  }

  startMetricPolls() {
    const queryList: MetricsPollingQueries = {
      queries: [],
      tenant: Utility.getInstance().getTenant()
    };
    queryList.queries.push(this.timeSeriesQuery());
    queryList.queries.push(this.avgQuery());
    queryList.queries.push(this.avgDayQuery());
    queryList.queries.push(this.clusterAvgQuery());
    const sub = this.metricsqueryService.pollMetrics('naplesDetailCards', queryList).subscribe(
      (data: IMetrics_queryQueryResponse) => {
        if (data && data.results && data.results.length === 4) {
          this.timeSeriesData = data.results[0];
          this.avgData = data.results[1];
          this.avgDayData = data.results[2];
          this.clusterAvgData = data.results[3];
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
    const query: Metrics_queryQuerySpec =
      MetricsUtility.timeSeriesQuery(this.telemetryKind, MetricsUtility.createNameSelector(this.selectedId));
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.timeSeriesQueryUpdate,
      mergeFunction: MetricsUtility.timeSeriesQueryMerge
    };

    return { query: query, pollingOptions: pollOptions };
  }

  avgQuery(): MetricsPollingQuery {
    const query: Metrics_queryQuerySpec = MetricsUtility.pastFiveMinAverageQuery(this.telemetryKind);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.pastFiveMinQueryUpdate,
    };

    return { query: query, pollingOptions: pollOptions };
  }

  avgDayQuery(): MetricsPollingQuery {
    const query: Metrics_queryQuerySpec = MetricsUtility.pastDayAverageQuery(this.telemetryKind);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.pastDayAverageQueryUpdate,
    };

    return { query: query, pollingOptions: pollOptions };
  }

  clusterAvgQuery(): MetricsPollingQuery {
    const clusterAvgQuery: IMetrics_queryQuerySpec = {
      'kind': this.telemetryKind,
      function: Metrics_queryQuerySpec_function.MEAN,
      // We don't specify the fields we need, as specifying more than one field
      // while using the average function isn't supported by the backend.
      // Instead we leave blank and get all fields
      fields: [],
      // We only look at the last 5 min bucket so that the max node reporting is never
      // lower than current
      'start-time': new Date(Utility.roundDownTime(5).getTime() - 1000 * 50 * 5).toISOString() as any,
      'end-time': Utility.roundDownTime(5).toISOString() as any
    };

    const query = new Metrics_queryQuerySpec(clusterAvgQuery);
    const timeUpdate = (queryBody: IMetrics_queryQuerySpec) => {
      queryBody['start-time'] = new Date(Utility.roundDownTime(5).getTime() - 1000 * 50 * 5).toISOString() as any,
        queryBody['end-time'] = Utility.roundDownTime(5).toISOString() as any;
    };

    const merge = (currData, newData) => {
      // since we round down to get the last 5 min bucket, there's a chance
      // that we can back null data, since no new metrics have been reported.
      // Data should have been filtered in metricsquery services's processData
      if (!MetricsUtility.resultHasData(newData)) {
        // no new data, keep old value
        return currData;
      }
      return newData;
    };

    const pollOptions = {
      timeUpdater: timeUpdate,
      mergeFunction: merge
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

  genCharts(fieldName, heroCard: HeroCardOptions) {
    // Time series graph
    if (MetricsUtility.resultHasData(this.timeSeriesData)) {
      const timeSeriesData = this.timeSeriesData;

      const index = timeSeriesData.series[0].columns.indexOf(fieldName);
      const data = Utility.transformToPlotly(timeSeriesData.series[0].values, 0, index);
      heroCard.data = data;
    }

    // Current stat calculation - we take the last point
    if (MetricsUtility.resultHasData(this.avgData)) {
      const index = this.avgData.series[0].columns.indexOf(fieldName);
      heroCard.firstStat.value = Math.round(this.avgData.series[0].values[0][index]) + '%';
    }

    // Avg
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

    // Cluster average
    if (this.clusterAvgData.series[0].values.length !== 0) {
      const index = this.clusterAvgData.series[0].columns.indexOf(fieldName);
      heroCard.thirdStat.value = Math.round(this.clusterAvgData.series[0].values[0][index]) + '%';
    }

    if (heroCard.cardState !== CardStates.READY) {
      heroCard.cardState = CardStates.READY;
    }
  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to be destroyed
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'sgpolicydetailComponent', 'state': Eventtypes.COMPONENT_DESTROY });
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

}
