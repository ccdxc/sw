import { Component, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { HeroCardOptions, StatArrowDirection } from '@app/components/shared/herocard/herocard.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MetricsPollingOptions, MetricsqueryService } from '@app/services/metricsquery.service';
import { ClusterSmartNIC } from '@sdk/v1/models/generated/cluster';
import { Metrics_queryQuerySpec } from '@sdk/v1/models/generated/metrics_query';
import { MessageService } from 'primeng/primeng';
import { Table } from 'primeng/table';
import { Subscription } from 'rxjs/Subscription';
import { IMetrics_queryQueryResponse } from '@sdk/v1/models/metrics_query';

@Component({
  selector: 'app-naples',
  encapsulation: ViewEncapsulation.None,
  templateUrl: './naples.component.html',
  styleUrls: ['./naples.component.scss']
})
export class NaplesComponent extends BaseComponent implements OnInit, OnDestroy {
  @ViewChild('naplesTable') naplesTurboTable: Table;

  naples: ReadonlyArray<ClusterSmartNIC> = [];
  // Used for processing the stream events
  naplesEventUtility: HttpEventUtility<ClusterSmartNIC>;

  cols: any[] = [
    { field: 'meta.name', header: 'Name', class: 'naples-column-date', sortable: false },
    { field: 'spec.hostname', header: 'Host name', class: 'naples-column-host-name', sortable: false },
    { field: 'spec.ip-config.ip-address', header: 'Management IP Address', class: 'naples-column-mgmt-cidr', sortable: false },
    { field: 'status.admission-phase', header: 'Phase', class: 'naples-column-phase', sortable: false },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'naples-column-date', sortable: false },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'naples-column-date', sortable: false },
  ];
  subscriptions: Subscription[] = [];

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px',
    },
    url: '/assets/images/icons/cluster/naples/ico-naples-black.svg',
  };

  cardColor = '#b592e3';

  cardIcon: Icon = {
    margin: {
      top: '10px',
      left: '10px'
    },
    svgIcon: 'naples'
  };

  cpuChartData: HeroCardOptions = MetricsUtility.clusterLevelCPUHeroCard(this.cardColor, this.cardIcon);

  memChartData: HeroCardOptions = MetricsUtility.clusterLevelMemHeroCard(this.cardColor, this.cardIcon);

  diskChartData: HeroCardOptions = MetricsUtility.clusterLevelDiskHeroCard(this.cardColor, this.cardIcon);

  heroCards = [
    this.cpuChartData,
    this.memChartData,
    this.diskChartData
  ];

  timeSeriesData: IMetrics_queryQueryResponse;
  avgData: IMetrics_queryQueryResponse;
  maxObjData: IMetrics_queryQueryResponse;

  telemetryKind: string = "SmartNIC";

  constructor(private clusterService: ClusterService,
    protected controllerService: ControllerService,
    protected metricsqueryService: MetricsqueryService,
    protected messageService: MessageService
  ) {
    super(controllerService, messageService);
  }

  ngOnInit() {
    this.getNaples();
    this.getMetrics();

    this.controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [{ label: 'Naples', url: Utility.getBaseUIUrl() + 'cluster/naples' }]
    });
  }

  getNaples() {
    this.naplesEventUtility = new HttpEventUtility<ClusterSmartNIC>(ClusterSmartNIC);
    this.naples = this.naplesEventUtility.array as ReadonlyArray<ClusterSmartNIC>;
    const subscription = this.clusterService.WatchSmartNIC().subscribe(
      response => {
        const body: any = response.body;
        this.naplesEventUtility.processEvents(body);
      },
      this.restErrorHandler('Failed to get NAPLES info')
    );
  }

  displayColumn(data, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
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
   * Naples overview level display
   *  - Time series graph of all the nodes averaged together, avg into 5 min buckets
   *  - Current avg of last 5 min
   *  - Average of past day
   *  - Naple using the most in the last 5 min
   */
  getMetrics() {
    this.timeSeriesQuery();
    this.avgQuery();
    this.maxNaplesQuery();
  }

  timeSeriesQuery() {
    const timeSeriesQuery: Metrics_queryQuerySpec = MetricsUtility.timeSeriesQuery(this.telemetryKind);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.timeSeriesQueryUpdate,
      mergeFunction: MetricsUtility.timeSeriesQueryMerge
    };

    const sub = this.metricsqueryService.pollMetrics('naplesOverviewTimeSeriesData', timeSeriesQuery, pollOptions).subscribe(
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

    const sub = this.metricsqueryService.pollMetrics('naplesOverviewAvgData', avgQuery, pollOptions).subscribe(
      (data) => {
        this.avgData = data;
        this.tryGenCharts();
      }
    );
    this.subscriptions.push(sub);
  }

  maxNaplesQuery() {
    const query: Metrics_queryQuerySpec = MetricsUtility.maxObjQuery(this.telemetryKind);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.maxObjQueryUpdate,
      mergeFunction: MetricsUtility.maxObjQueryMerge
    };

    const sub = this.metricsqueryService.pollMetrics('naplesOverviewMaxObjData', query, pollOptions).subscribe(
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
      this.genCharts('mean_CPUUsedPercent', this.cpuChartData);
      this.genCharts('mean_MemUsedPercent', this.memChartData);
      this.genCharts('mean_DiskUsedPercent', this.diskChartData);
    }
  }

  private genCharts(fieldName: string, heroCard: HeroCardOptions) {
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

    // Max Naples
    const maxNaples = MetricsUtility.maxObjUtility(this.maxObjData, fieldName);
    if (maxNaples == null || maxNaples.max === -1) {
      heroCard.thirdStat.value = null;
    } else {
      // Removing SmartNIC- prefix from the name and adding value
      const thirdStatName = maxNaples.name.substring(this.telemetryKind.length + 1)
      let thirdStat = thirdStatName;
      if (thirdStat.length > 10) {
        thirdStat = thirdStat.substring(0, 11) + '...';
      }
      thirdStat += ' (' + Math.round(maxNaples.max) + '%)';
      heroCard.thirdStat.value = thirdStat;
      heroCard.thirdStat.url = '/cluster/naples/' + thirdStatName;
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
