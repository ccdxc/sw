import { Component, Input, OnInit, ViewEncapsulation, ViewChild } from '@angular/core';
import { OnChanges, OnDestroy, AfterViewInit } from '@angular/core/src/metadata/lifecycle_hooks';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { LineGraphStat, LinegraphComponent } from '@app/components/shared/linegraph/linegraph.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ChartOptions, ChartData } from 'chart.js';
import { StatArrowDirection, CardStates, Stat } from '@app/components/shared/basecard/basecard.component';
import { FlipState, FlipComponent } from '@app/components/shared/flip/flip.component';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { ClusterDistributedServiceCard, ClusterDistributedServiceCardStatus_admission_phase, IClusterDistributedServiceCardList } from '@sdk/v1/models/generated/cluster';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ControllerService } from '@app/services/controller.service';
import { Subscription } from 'rxjs';
import { TelemetryPollingMetricQueries, MetricsqueryService, MetricsPollingQuery, MetricsPollingOptions } from '@app/services/metricsquery.service';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { ITelemetry_queryMetricsQueryResponse, ITelemetry_queryMetricsQueryResult } from '@sdk/v1/models/telemetry_query';
import { NaplesConditionValues} from '@app/components/cluster-group/naples/index.ts';
import { Telemetry_queryMetricsQuerySpec, Telemetry_queryMetricsQuerySpec_function } from '@sdk/v1/models/generated/telemetry_query';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import {Router} from '@angular/router';

@Component({
  selector: 'app-dsbdnapleswidget',
  templateUrl: './naples.component.html',
  styleUrls: ['./naples.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NaplesComponent implements OnInit, OnChanges, AfterViewInit, OnDestroy {
  @ViewChild('lineGraph') lineGraphComponent: LinegraphComponent;
  subscriptions: Subscription[] = [];
  naples: ReadonlyArray<ClusterDistributedServiceCard> = [];
  // Used for processing the stream events
  naplesEventUtility: HttpEventUtility<ClusterDistributedServiceCard>;

  hasHover: boolean = false;
  cardStates = CardStates;

  title: string = 'Distributed Services Cards';
  firstStat: Stat = {
    value: '',
    description: 'TOTAL DSC',
    arrowDirection: StatArrowDirection.HIDDEN,
    statColor: '#b592e3'
  };
  secondStat: Stat = {
    value: '',
    description: 'ADMITTED',
    arrowDirection: StatArrowDirection.HIDDEN,
    statColor: '#b592e3'
  };
  thirdStat: Stat = {
    value: '',
    description: 'REJECTED',
    arrowDirection: StatArrowDirection.HIDDEN,
    statColor: '#e57553'
  };
  fourthStat: Stat = {
    value: '',
    description: 'PENDING',
    arrowDirection: StatArrowDirection.HIDDEN,
    statColor: '#97b8df'
  };


  totalNaplesStat: LineGraphStat = {
    title: 'ADMITTED DSC',
    data: [],
    statColor: '#b592e3',
    gradientStart: 'rgba(181,146, 227,1)',
    gradientStop: 'rgba(181,146, 227,0)',
    graphId: 'dsbdnaples-totalNaples',
    defaultValue: 0,
    defaultDescription: 'Avg',
    hoverDescription: 'DSCs',  // VS-744 , make the label short.
    isPercentage: false,
    scaleMin: 0,
  };
  rejectedNaplesStat: LineGraphStat = {
    title: 'REJECTED DSC',
    data: [],
    statColor: '#e57553',
    gradientStart: 'rgba(229, 117, 83, 1)',
    gradientStop: 'rgba(229, 117, 83, 0)',
    graphId: 'dsbdnaples-rejectedNaples',
    defaultValue: 0,
    defaultDescription: 'Avg',
    hoverDescription: 'DSCs',
    isPercentage: false,
    scaleMin: 0,
  };
  pendingNaplesStat: LineGraphStat = {
    title: 'PENDING DSC',
    data: [],
    statColor: '#97b8df',
    gradientStart: 'rgba(151, 184, 223, 1)',
    gradientStop: 'rgba(151, 184, 223, 0)',
    graphId: 'dsbdnaples-pendingNaples',
    defaultValue: 0,
    defaultDescription: 'Avg',
    hoverDescription: 'DSCs',
    isPercentage: false,
    scaleMin: 0,
  };

  linegraphStats: LineGraphStat[] = [
    this.totalNaplesStat,
    this.rejectedNaplesStat,
    this.pendingNaplesStat
  ];

  healthyNaplesCount = 0;
  unknownNaplesCount = 0;

  themeColor: string = '#b592e3';
  backgroundIcon: Icon = {
    svgIcon: 'naples',
    margin: {}
  };
  icon: Icon = {
    margin: {
      top: '10px',
      left: '10px'
    },
    svgIcon: 'naples'
  };
  @Input() lastUpdateTime: string;
  @Input() timeRange: string;
  // When set to true, card contents will fade into view
  frontCardState: CardStates = CardStates.LOADING;

  backCardState: CardStates = CardStates.LOADING;

  flipState: FlipState = FlipState.front;

  menuItems = [

  ];


  showGraph: boolean = false;

  healthyPercent: number;
  unhealthyPercent: number;
  unknownPercent: number;

  dataDoughnut: ChartData;

  options: ChartOptions;

  metricData: ITelemetry_queryMetricsQueryResult;
  metricAvgData: ITelemetry_queryMetricsQueryResult;

  graphDrawn: boolean = false;
  viewInitialized: boolean = false;

  pieChartText: string = '';
  pieChartPercent: string = '';

  constructor(private controllerService: ControllerService,
              protected metricsqueryService: MetricsqueryService,
              protected uiconfigsService: UIConfigsService,
              protected router: Router,
    protected clusterService: ClusterService) { }

  toggleFlip() {
    // The trend line metrics requires cluster read permissions
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.clustercluster_read)) {
      this.flipState = FlipComponent.toggleState(this.flipState);
    }
  }

  generateDoughnut() {
    if (this.healthyNaplesCount == null || this.unhealthyPercent == null) {
      this.dataDoughnut = null;
      return;
    }
    this.dataDoughnut =  {
      labels: ['Healthy', 'Unhealthy', 'Unknown'],
      datasets: [
        {
          data: [this.healthyPercent, this.unhealthyPercent, this.unknownPercent],
          backgroundColor: [
            '#97b8df',
            '#e57553',
            '#ffe4b5'
          ],
        }
      ]
    };
    this.options =  {
      tooltips: {
        enabled: true,
        displayColors: false,
        titleFontFamily: 'Fira Sans Condensed',
        titleFontSize: 14,
        bodyFontFamily: 'Fira Sans Condensed',
        bodyFontSize: 13,
        callbacks: {
          label: function(tooltipItem, data) {
            const dataset = data.datasets[tooltipItem.datasetIndex];
            const label = data.labels[tooltipItem.index];
            const val: any = dataset.data[tooltipItem.index];
            const rounded: any = Math.round(val * 10) / 10;
            if (label === 'Healthy') {
              return rounded + '% of DSCs are healthy';
            } else if (label === 'Unhealthy') {
              return rounded + '% of DSCs have critical errors';
            } else {
              return rounded + '% of NSCs are not reachable';
            }
          }
        }
      },
      title: {
        display: false
      },
      legend: {
        display: false
      },
      cutoutPercentage: 60,
      circumference: 2 * Math.PI,
      rotation: (1.0 + this.unhealthyPercent / 100) * Math.PI, // work

      plugins: {
        datalabels: {
          backgroundColor: function(context) {
            return context.dataset.backgroundColor;
          },
          borderColor: 'white',
          borderRadius: 25,
          borderWidth: 2,
          color: 'white',
          display: function(context) {
            // print bad % only
            return context.dataIndex > 0;
          },
          font: {
            weight: 'bold',
            family: 'Fira Sans Condensed'
          },
          formatter: Math.round
        }
      },
      animation: {
        duration: 0,
      }
    };
  }

  export() {
    const exportObj = {
      naples: this.naples,
      admitted: this.secondStat.value,
      rejected: this.thirdStat.value,
      pending: this.fourthStat.value,
      percentHealthy: this.healthyPercent,
      totalNaplesGraph: this.totalNaplesStat.data,
      pendingNaplesGraph: this.pendingNaplesStat.data,
      rejectedNaplesGraph: this.rejectedNaplesStat.data,
    };
    const fieldName = 'naples-dataset.json';
    Utility.exportContent(JSON.stringify(exportObj, null, 2), 'text/json;charset=utf-8;', fieldName);
    Utility.getInstance().getControllerService().invokeInfoToaster('Data exported', 'Please find ' + fieldName + ' in your download folder');
  }

  ngOnChanges(changes) {
  }

  ngAfterViewInit() {
    this.viewInitialized = true;
    this.tryGenMetrics();
  }

  ngOnInit() {
    this.getNaples();
    this.getMetrics();
    // Linegraph data needs to read from cluster measurement
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.clustercluster_read)) {
      this.menuItems.push(
        {
          text: 'Flip card', onClick: () => {
            this.toggleFlip();
          }
        }
      );
      this.menuItems.push(
        {
          text: 'Export', onClick: () => {
            this.export();
          }
        }
      );
      this.menuItems.push(
        {
          text: 'Navigate to DSCs', onClick: () => {
            this.goToNaples();
          }
        }
      );
    }
  }

  goToNaples() {
    this.router.navigateByUrl('/cluster/dscs');
  }

  tryGenMetrics() {
    if (this.viewInitialized && this.backCardState === CardStates.READY) {
      this.setupCardBack();
    }
  }

  getMetrics() {
    const queryList: TelemetryPollingMetricQueries = {
      queries: [],
      tenant: Utility.getInstance().getTenant()
    };
    const timeQuery = MetricsUtility.timeSeriesQueryPolling('Cluster', []);
    timeQuery.query.function = Telemetry_queryMetricsQuerySpec_function.last; // VS-790 user last func instead o f median function
    queryList.queries.push(timeQuery);
    queryList.queries.push(this.avgQuery());

    const sub = this.metricsqueryService.pollMetrics('naplesHealthCards', queryList).subscribe(
      (data: ITelemetry_queryMetricsQueryResponse) => {
        if (data && data.results && data.results.length === queryList.queries.length) {
          if (MetricsUtility.resultHasData(data.results[0])) {
            this.metricData = data.results[0];
            this.metricAvgData = data.results[1];
            this.lastUpdateTime = new Date().toISOString();
            this.backCardState = CardStates.READY;
            this.tryGenMetrics();
          } else {
            this.backCardState = CardStates.NO_DATA;
          }
        }
      },
      (err) => {
        this.backCardState = CardStates.FAILED;
      }
    );
    this.subscriptions.push(sub);
  }

  avgQuery(): MetricsPollingQuery {
    const query: Telemetry_queryMetricsQuerySpec = MetricsUtility.pastDayAverageQuery('Cluster');
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.pastDayAverageQueryUpdate,
    };
    return { query: query, pollingOptions: pollOptions };
  }

  setupCardBack() {
    if (MetricsUtility.resultHasData(this.metricData)) {
      let data = MetricsUtility.transformToChartjsTimeSeries(this.metricData.series[0], 'AdmittedNICs');
      this.totalNaplesStat.data = data;

      data = MetricsUtility.transformToChartjsTimeSeries(this.metricData.series[0], 'RejectedNICs');
      this.rejectedNaplesStat.data = data;

      data = MetricsUtility.transformToChartjsTimeSeries(this.metricData.series[0], 'PendingNICs');
      this.pendingNaplesStat.data = data;
    }

    if (MetricsUtility.resultHasData(this.metricAvgData)) {
      let index = MetricsUtility.findFieldIndex(this.metricAvgData.series[0].columns, 'AdmittedNICs');
      this.totalNaplesStat.defaultValue = Math.round(this.metricAvgData.series[0].values[0][index]);

      index = MetricsUtility.findFieldIndex(this.metricAvgData.series[0].columns, 'RejectedNICs');
      this.rejectedNaplesStat.defaultValue = Math.round(this.metricAvgData.series[0].values[0][index]);

      index = MetricsUtility.findFieldIndex(this.metricAvgData.series[0].columns, 'PendingNICs');
      this.pendingNaplesStat.defaultValue = Math.round(this.metricAvgData.series[0].values[0][index]);
    }
    if (this.graphDrawn) {
      // Manually calling setup charts to redraw as default
      // change detection will not trigger when the data changes
      this.lineGraphComponent.setupCharts();
    } else {
      this.graphDrawn = true;
      // In case we just switched from loading state to ready state,
      // we need to wait for dom to render the canvas
      setTimeout(() => {
        // change detection will not trigger when the data changes
        this.lineGraphComponent.setupCharts();
      }, 0);
    }
  }

  getNaples() {
    // We perform a get as well as a watch so that we can know to set the card state
    // in case the number of workloads is 0
    this.clusterService.ListDistributedServiceCard().subscribe(
      (resp) => {
        const body: IClusterDistributedServiceCardList = resp.body as any;
        if (body.items == null || body.items.length === 0) {
          // Watch won't have any events
          this.calculateNaplesStatus();
          this.frontCardState = CardStates.READY;
        }
      },
      error => {
        this.frontCardState = CardStates.FAILED;
      }
    );
    this.naplesEventUtility = new HttpEventUtility<ClusterDistributedServiceCard>(ClusterDistributedServiceCard);
    this.naples = this.naplesEventUtility.array as ReadonlyArray<ClusterDistributedServiceCard>;
    const subscription = this.clusterService.WatchDistributedServiceCard().subscribe(
      response => {
        this.naplesEventUtility.processEvents(response);
        this.calculateNaplesStatus();
        this.frontCardState = CardStates.READY;
      },
      (err) => {
        this.frontCardState = CardStates.FAILED;
      }
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  calculateNaplesStatus() {
    let rejected = 0; let admitted = 0; let pending = 0;
    this.healthyNaplesCount = 0;
    this.unknownNaplesCount = 0;
    const admittedNics = [];
    this.naples.forEach((naple) => {
      if (Utility.isNaplesNICHealthy(naple)) {
        this.healthyNaplesCount += 1;
      } else if (Utility.getNaplesCondition(naple) === NaplesConditionValues.UNKNOWN) {
        this.unknownNaplesCount += 1;
      }
      if (!Utility.isNICConditionEmpty(naple)) {
        admittedNics.push(naple);
      }
      switch (naple.status['admission-phase']) {
        case ClusterDistributedServiceCardStatus_admission_phase.admitted:
          admitted += 1;
          break;
        case ClusterDistributedServiceCardStatus_admission_phase.rejected:
          rejected += 1;
          break;
        case ClusterDistributedServiceCardStatus_admission_phase.pending:
          pending += 1;
          break;
      }
    });
    this.firstStat.value = this.naples.length.toString();
    this.secondStat.value = admitted.toString();
    this.thirdStat.value = rejected.toString();
    this.fourthStat.value = pending.toString();
    if (admittedNics && admittedNics.length !== 0) {
      const unhealthyCount = admittedNics.length - this.healthyNaplesCount - this.unknownNaplesCount;
      this.unhealthyPercent = (unhealthyCount / admittedNics.length) * 100;
      this.unknownPercent = (this.unknownNaplesCount / admittedNics.length) * 100;
      // Calculate healthy last so any rounding error will lower the health percent
      this.healthyPercent = 100 - this.unhealthyPercent - this.unknownPercent;
    } else {
      this.healthyPercent = null;
      this.unhealthyPercent = null;
      this.unknownPercent = null;
    }
    this.generatePieChartText();
    this.generateDoughnut();
  }

  generatePieChartText() {
    if (this.healthyPercent == null) {
      return '';
    }
    this.pieChartPercent = this.roundHealthNumber() + '%';
    this.pieChartText = 'Healthy';
  }

  roundHealthNumber(): number {
    const num: number = this.healthyPercent;
    if (num >= 100) {
      return 100;
    }
    const roundNum = Math.round(num * 100);
    // never all 100% if there is a single unhelth card
    if (num >= 10000) {
      return 99.99;
    }
    return roundNum / 100;
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }
}
