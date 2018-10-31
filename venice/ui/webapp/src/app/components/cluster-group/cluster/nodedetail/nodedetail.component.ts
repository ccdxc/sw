import { Component, OnInit } from '@angular/core';
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
import { HeroCardOptions, StatArrowDirection } from '@app/components/shared/herocard/herocard.component';
import { IMetrics_queryQueryResponse } from '@sdk/v1/models/metrics_query';
import { Metrics_queryQuerySpec, IMetrics_queryQuerySpec, Metrics_queryQuerySpec_function } from '@sdk/v1/models/generated/metrics_query';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { MetricsPollingOptions, MetricsqueryService } from '@app/services/metricsquery.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';

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
export class NodedetailComponent extends BaseComponent implements OnInit {
  subscriptions = [];

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/cluster/nodes/ico-node-black.svg'
  };

  nodeCardColor = '#61b3a0';
  nodeIcon: Icon = {
    margin: {
      top: '10px',
      left: '10px'
    },
    svgIcon: 'node'
  };

  // Id of the node the user has navigated to
  selectedNodeId: string;
  selectedNode: ClusterNode;

  //Holds all nodes, should be only one item in the array
  nodesList: ReadonlyArray<ClusterNode>;
  nodesEventUtility: HttpEventUtility<ClusterNode>;

  // Whether we show a deletion overlay
  showDeletionScreen: boolean;

  // Whether we show a missing overlay
  showMissingScreen: boolean;

  cpuChartData: HeroCardOptions = {
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
    themeColor: this.nodeCardColor,
    icon: this.nodeIcon
  };

  memChartData: HeroCardOptions = {
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
    themeColor: this.nodeCardColor,
    icon: this.nodeIcon
  };

  diskChartData: HeroCardOptions = {
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
    themeColor: this.nodeCardColor,
    icon: this.nodeIcon
  };
  heroCards = [
    this.cpuChartData,
    this.memChartData,
    this.diskChartData
  ];

  timeSeriesData: IMetrics_queryQueryResponse;
  avgData: IMetrics_queryQueryResponse;
  clusterAvgData: IMetrics_queryQueryResponse;

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
    this._route.params
      .map(params => params['id'])
      .subscribe((id) => {
        this.selectedNodeId = id;
        this.initializeData();
        this.getNodedetails();
        this._controllerService.setToolbarData({
          buttons: [],
          breadcrumb: [
            { label: 'Cluster', url: Utility.getBaseUIUrl() + 'cluster/cluster' },
            { label: id }]
        });
      });
  }

  initializeData() {
    // Initializing variables so that state is cleared between routing of different
    // sgpolicies
    // Ex. /cluster/node1-> /cluster/node2
    this.nodesList = [];
    this.showDeletionScreen = false;
    this.showMissingScreen = false;
    this.selectedNode = null;
    this.timeSeriesData = null;
    this.avgData = null;
    this.clusterAvgData = null;
    this.nodesList = [];
    this.selectedNode = null;

    this.heroCards.forEach((card) => {
      card.firstStat.value = null;
      card.secondStat.value = null;
      card.thirdStat.value = null;
      card.data = { x: [], y: [] };
      card.isReady = false;
    });
  }

  getNodedetails() {
    // We perform a get as well as a watch so that we can know if the object the user is
    // looking for exists or not.
    const getSubscription = this.clusterService.GetNode(this.selectedNodeId).subscribe(
      response => {
        // We do nothing, and wait for the callback of the watch to populate the view
      },
      error => {
        // If we receive any error code we display object is missing
        // TODO: Update to be more descriptive based on error message
        this.showMissingScreen = true;
        this.heroCards.forEach(card => {
          card.isReady = true;
        });
      }
    );
    this.nodesEventUtility = new HttpEventUtility<ClusterNode>(ClusterNode);
    this.nodesList = this.nodesEventUtility.array;
    const subscription = this.clusterService.WatchNode({ 'field-selector': 'ObjectMeta.Name=' + this.selectedNodeId }).subscribe(
      response => {
        const body: any = response.body;
        this.nodesEventUtility.processEvents(body);
        if (this.nodesList.length > 1) {
          // because of the name selector, we should
          // have only got one object
          console.error(
            'Received more than one node. Expected '
            + this.selectedNodeId + ', received ' +
            this.nodesList.map((policy) => policy.meta.name).join(', '));
        }
        if (this.nodesList.length > 0) {
          // In case object was deleted and then readded while we are on the same screen
          this.showDeletionScreen = false;
          // In case object wasn't created yet and then was added while we are on the same screen
          this.showMissingScreen = false;
          this.selectedNode = this.nodesList[0];
          this.startMetricPolls();
        } else {
          // Must have received a delete event.
          this.showDeletionScreen = true;
          this.heroCards.forEach(card => {
            card.isReady = true;
          });
          this.selectedNode = null;
        }
      },
      this.restErrorHandler('Failed to get Node ' + this.selectedNodeId)
    );
    this.subscriptions.push(subscription);
  }

  startMetricPolls() {
    this.timeSeriesQuery();
    this.avgQuery();
    this.clusterAvgQuery();
  }

  timeSeriesQuery() {
    const timeSeriesQuery: Metrics_queryQuerySpec =
      MetricsUtility.timeSeriesQuery('Node', MetricsUtility.createNameSelector(this.selectedNodeId));
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
    const avgQuery: Metrics_queryQuerySpec =
      MetricsUtility.pastDayAverageQuery('Node', MetricsUtility.createNameSelector(this.selectedNodeId));
    const pollOptions = {
      timeUpdater: MetricsUtility.pastDayAverageQueryUpdate,
    };

    const sub = this.metricsqueryService.pollMetrics('avgData', avgQuery, pollOptions).subscribe(
      (data) => {
        this.avgData = data;
        this.tryGenCharts();
      }
    );
    this.subscriptions.push(sub);
  }

  clusterAvgQuery() {
    const clusterAvgQuery: IMetrics_queryQuerySpec = {
      'kind': 'Node',
      'meta': {
        'tenant': Utility.getInstance().getTenant()
      },
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
      if (!MetricsUtility.hasData(newData)) {
        // no new data, keep old value
        return currData;
      }
      return newData;
    };

    const pollOptions = {
      timeUpdater: timeUpdate,
      mergeFunction: merge
    };

    const sub = this.metricsqueryService.pollMetrics('clusterAvgQuery', clusterAvgQuery, pollOptions).subscribe(
      (data) => {
        this.clusterAvgData = data;
        this.tryGenCharts();
      }
    );
    this.subscriptions.push(sub);
  }

  private tryGenCharts() {
    if (MetricsUtility.hasData(this.timeSeriesData) &&
      MetricsUtility.hasData(this.avgData) &&
      MetricsUtility.hasData(this.clusterAvgData)) {
      this.genChart('mean_CPUUsedPercent', this.cpuChartData);
      this.genChart('mean_MemUsedPercent', this.memChartData);
      this.genChart('mean_DiskUsedPercent', this.diskChartData);
    }
  }

  genChart(fieldName, heroCard: HeroCardOptions) {
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

    // Cluster average
    const clusterAvg = this.clusterAvgData;
    if (clusterAvg.results[0].series[0].values.length !== 0) {
      heroCard.thirdStat.value = Math.round(clusterAvg.results[0].series[0].values[0][index]) + '%';
    }

    if (!heroCard.isReady) {
      heroCard.isReady = true;
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
