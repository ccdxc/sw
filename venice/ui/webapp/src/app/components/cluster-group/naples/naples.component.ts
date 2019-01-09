import { Component, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { HeroCardOptions } from '@app/components/shared/herocard/herocard.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MetricsPollingOptions, MetricsqueryService, MetricsPollingQueries, MetricsPollingQuery } from '@app/services/metricsquery.service';
import { ClusterSmartNIC } from '@sdk/v1/models/generated/cluster';
import { Metrics_queryQuerySpec } from '@sdk/v1/models/generated/metrics_query';
import { MessageService } from 'primeng/primeng';
import { Table } from 'primeng/table';
import { Subscription } from 'rxjs';
import { IMetrics_queryQueryResponse, IMetrics_queryQueryResult } from '@sdk/v1/models/metrics_query';
import { StatArrowDirection, CardStates } from '@app/components/shared/basecard/basecard.component';

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
    { field: 'meta.name', header: 'Name', class: 'naples-column-date', sortable: true },
    { field: 'spec.hostname', header: 'Host name', class: 'naples-column-host-name', sortable: true },
    { field: 'spec.ip-config.ip-address', header: 'Management IP Address', class: 'naples-column-mgmt-cidr', sortable: false },
    { field: 'status.admission-phase', header: 'Phase', class: 'naples-column-phase', sortable: false },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'naples-column-date', sortable: true },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'naples-column-date', sortable: true },
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

  timeSeriesData: IMetrics_queryQueryResult;
  avgData: IMetrics_queryQueryResult;
  avgDayData: IMetrics_queryQueryResult;
  maxObjData: IMetrics_queryQueryResult;

  telemetryKind: string = 'SmartNIC';

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
        this.naplesEventUtility.processEvents(response);
      },
      this.restErrorHandler('Failed to get NAPLES info')
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
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
    const queryList: MetricsPollingQueries = {
      queries: [],
      tenant: Utility.getInstance().getTenant()
    };
    queryList.queries.push(this.timeSeriesQuery());
    queryList.queries.push(this.avgQuery());
    queryList.queries.push(this.avgDayQuery());
    queryList.queries.push(this.maxNaplesQuery());

    const sub = this.metricsqueryService.pollMetrics('naplesCards', queryList).subscribe(
      (data: IMetrics_queryQueryResponse) => {
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
    const query: Metrics_queryQuerySpec = MetricsUtility.timeSeriesQuery(this.telemetryKind);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.timeSeriesQueryUpdate,
      mergeFunction: MetricsUtility.timeSeriesQueryMerge
    };
    return { query: query, pollingOptions: pollOptions };
  }

  avgQuery(): MetricsPollingQuery {
    const query: Metrics_queryQuerySpec = MetricsUtility.pastDayAverageQuery(this.telemetryKind);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.pastDayAverageQueryUpdate,
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

  maxNaplesQuery(): MetricsPollingQuery {
    const query: Metrics_queryQuerySpec = MetricsUtility.maxObjQuery(this.telemetryKind);
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

  private genCharts(fieldName: string, heroCard: HeroCardOptions) {
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

    // Max naples
    if (MetricsUtility.resultHasData(this.maxObjData)) {
      const maxNaples = MetricsUtility.maxObjUtility(this.maxObjData, fieldName);
      if (maxNaples == null || maxNaples.max === -1) {
        heroCard.thirdStat.value = null;
      } else {
        const thirdStatName = maxNaples.name;
        let thirdStat = thirdStatName;
        if (thirdStat.length > 10) {
          thirdStat = thirdStat.substring(0, 11) + '...';
        }
        thirdStat += ' (' + Math.round(maxNaples.max) + '%)';
        heroCard.thirdStat.value = thirdStat;
        heroCard.thirdStat.url = '/cluster/naples/' + thirdStatName;
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
