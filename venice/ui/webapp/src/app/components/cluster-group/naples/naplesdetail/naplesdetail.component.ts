import { Component, OnInit, OnDestroy } from '@angular/core';
import { BaseComponent } from '@app/components/base/base.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ClusterSmartNIC } from '@sdk/v1/models/generated/cluster';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { HeroCardOptions, StatArrowDirection } from '@app/components/shared/herocard/herocard.component';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { IMetrics_queryQueryResponse } from '@sdk/v1/models/metrics_query';
import { ControllerService } from '@app/services/controller.service';
import { ActivatedRoute } from '@angular/router';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MetricsqueryService, MetricsPollingOptions } from '@app/services/metricsquery.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { MessageService } from 'primeng/primeng';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Utility } from '@app/common/Utility';
import { Metrics_queryQuerySpec, IMetrics_queryQuerySpec, Metrics_queryQuerySpec_function } from '@sdk/v1/models/generated/metrics_query';
import { AlertsEventsSelector } from '@app/components/shared/alertsevents/alertsevents.component';
import { map } from 'rxjs/internal/operators';

@Component({
  selector: 'app-naplesdetail',
  templateUrl: './naplesdetail.component.html',
  styleUrls: ['./naplesdetail.component.scss']
})
export class NaplesdetailComponent extends BaseComponent implements OnInit, OnDestroy {
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
    svgIcon: 'naples'
  };

  // Id of the object the user has navigated to
  selectedId: string;
  selectedObj: ClusterSmartNIC;

  // Holds all objects, should be only one item in the array
  objList: ReadonlyArray<ClusterSmartNIC>;
  objEventUtility: HttpEventUtility<ClusterSmartNIC>;

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

  timeSeriesData: IMetrics_queryQueryResponse;
  avgData: IMetrics_queryQueryResponse;
  clusterAvgData: IMetrics_queryQueryResponse;

  alertseventsSelector: AlertsEventsSelector;

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
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'NapledetailComponent', 'state': Eventtypes.COMPONENT_INIT });
    this._route.paramMap.subscribe(params => {
      const id = params.get('id');
      this.selectedId = id;
      this.initializeData();
      this.getNaplesDetails();
      this._controllerService.setToolbarData({
        buttons: [],
        breadcrumb: [
          { label: 'Naples', url: Utility.getBaseUIUrl() + 'cluster/naples' },
          { label: id, url: Utility.getBaseUIUrl() + 'cluster/naples/' + id }]
      });
    });
  }

  initializeData() {
    // Initializing variables so that state is cleared between routing of different
    // sgpolicies
    // Ex. /cluster/naples/naples1-> /cluster/naples/naples2
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
    this.clusterAvgData = null;
    this.objList = [];
    this.selectedObj = null;
    this.alertseventsSelector = null;

    this.heroCards.forEach((card) => {
      card.firstStat.value = null;
      card.secondStat.value = null;
      card.thirdStat.value = null;
      card.data = { x: [], y: [] };
      card.isReady = false;
    });
  }

  getNaplesDetails() {
    // We perform a get as well as a watch so that we can know if the object the user is
    // looking for exists or not.
    // Adding ':' as a temporary workaround of ApiGw not being able to
    // correctly parse smartNic names without it.
    const getSubscription = this.clusterService.GetSmartNIC(this.selectedId + ':').subscribe(
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
    this.subscriptions.push(getSubscription);
    this.objEventUtility = new HttpEventUtility<ClusterSmartNIC>(ClusterSmartNIC);
    this.objList = this.objEventUtility.array;
    const subscription = this.clusterService.WatchSmartNIC({ 'field-selector': 'meta.name=' + this.selectedId }).subscribe(
      response => {
        this.objEventUtility.processEvents(response);
        if (this.objList.length > 1) {
          // because of the name selector, we should
          // have only got one object
          console.error(
            'Received more than one naples object. Expected '
            + this.selectedId + ', received ' +
            this.objList.map((naples) => naples.meta.name).join(', '));
        }
        if (this.selectedObj == null && this.objList.length > 0) {
          // In case object was deleted and then readded while we are on the same screen
          this.showDeletionScreen = false;
          // In case object wasn't created yet and then was added while we are on the same screen
          this.showMissingScreen = false;
          this.selectedObj = this.objList[0];
          this.alertseventsSelector = {
            eventSelector: 'object-ref.name=' + this.selectedId + ',object-ref.kind=SmartNIC',
            alertSelector: 'status.object-ref.name=' + this.selectedId + ',status.object-ref.kind=SmartNIC',
          };
          this.startMetricPolls();
        } else if (this.objList.length === 0) {
          // Must have received a delete event.
          this.showDeletionScreen = true;
          this.heroCards.forEach(card => {
            card.isReady = true;
          });
          this.selectedObj = null;
        }
      },
      this.restErrorHandler('Failed to get Naples ' + this.selectedId)
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
      MetricsUtility.timeSeriesQuery('SmartNIC', MetricsUtility.createNameSelector(this.selectedId));
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
      MetricsUtility.pastDayAverageQuery('SmartNIC', MetricsUtility.createNameSelector(this.selectedId));
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
      'kind': 'SmartNIC',
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
