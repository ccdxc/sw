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
import { ClusterSmartNIC, ClusterSmartNICStatus_admission_phase } from '@sdk/v1/models/generated/cluster';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ControllerService } from '@app/services/controller.service';
import { Subscription } from 'rxjs';
import { TelemetryPollingMetricQueries, MetricsqueryService, MetricsPollingQuery, MetricsPollingOptions } from '@app/services/metricsquery.service';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { ITelemetry_queryMetricsQueryResponse, ITelemetry_queryMetricsQueryResult } from '@sdk/v1/models/telemetry_query';
import { NaplesConditionValues} from '@app/components/cluster-group/naples/index.ts';
import { Telemetry_queryMetricsQuerySpec, Telemetry_queryMetricsQuerySpec_function } from '@sdk/v1/models/generated/telemetry_query';

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
  naples: ReadonlyArray<ClusterSmartNIC> = [];
  // Used for processing the stream events
  naplesEventUtility: HttpEventUtility<ClusterSmartNIC>;

  hasHover: boolean = false;
  cardStates = CardStates;

  title: string = 'Naples ';
  firstStat: Stat = {
    value: '',
    description: 'TOTAL NAPLES',
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
    title: 'ADMITTED NAPLES',
    data: [],
    statColor: '#b592e3',
    gradientStart: 'rgba(181,146, 227,1)',
    gradientStop: 'rgba(181,146, 227,0)',
    graphId: 'dsbdnaples-totalNaples',
    defaultValue: 0,
    defaultDescription: 'Avg',
    hoverDescription: 'Naples',
    isPercentage: false,
    scaleMin: 0,
  };
  rejectedNaplesStat: LineGraphStat = {
    title: 'REJECTED NAPLES',
    data: [],
    statColor: '#e57553',
    gradientStart: 'rgba(229, 117, 83, 1)',
    gradientStop: 'rgba(229, 117, 83, 0)',
    graphId: 'dsbdnaples-rejectedNaples',
    defaultValue: 0,
    defaultDescription: 'Avg',
    hoverDescription: 'Naples',
    isPercentage: false,
    scaleMin: 0,
  };
  pendingNaplesStat: LineGraphStat = {
    title: 'PENDING NAPLES',
    data: [],
    statColor: '#97b8df',
    gradientStart: 'rgba(151, 184, 223, 1)',
    gradientStop: 'rgba(151, 184, 223, 0)',
    graphId: 'dsbdnaples-pendingNaples',
    defaultValue: 0,
    defaultDescription: 'Avg',
    hoverDescription: 'Naples',
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
    {
      text: 'Flip card', onClick: () => {
        this.toggleFlip();
      }
    },
    {
      text: 'Export', onClick: () => {
        this.export();
      }
    }
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
    protected clusterService: ClusterService) { }

  toggleFlip() {
    this.flipState = FlipComponent.toggleState(this.flipState);
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
              return rounded + '% of Naples are healthy';
            } else if (label === 'Unhealthy') {
              return rounded + '% of Naples have critical errors';
            } else {
              return rounded + '% of Naples are not reachable';
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
    const timeQuery = MetricsUtility.timeSeriesQueryPolling('Cluster');
    timeQuery.query.function = Telemetry_queryMetricsQuerySpec_function.MEDIAN;
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
    this.naplesEventUtility = new HttpEventUtility<ClusterSmartNIC>(ClusterSmartNIC);
    this.naples = this.naplesEventUtility.array as ReadonlyArray<ClusterSmartNIC>;
    const subscription = this.clusterService.WatchSmartNIC().subscribe(
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
    this.naples.forEach((naple) => {
      if (Utility.isNaplesNICHealthy(naple)) {
        this.healthyNaplesCount += 1;
      } else if (Utility.getNaplesCondition(naple) === NaplesConditionValues.UNKNOWN) {
        this.unknownNaplesCount += 1;
      }
      switch (naple.status['admission-phase']) {
        case ClusterSmartNICStatus_admission_phase.ADMITTED:
          admitted += 1;
          break;
        case ClusterSmartNICStatus_admission_phase.REJECTED:
          rejected += 1;
          break;
        case ClusterSmartNICStatus_admission_phase.PENDING:
          pending += 1;
          break;
      }
    });
    this.firstStat.value = this.naples.length.toString();
    this.secondStat.value = admitted.toString();
    this.thirdStat.value = rejected.toString();
    this.fourthStat.value = pending.toString();
    if (this.naples.length !== 0) {
      // Using floor instead of round so that even if
      // it is 99.5% healthy, we show 1% error
      // to alert the user
      this.healthyPercent = (this.healthyNaplesCount / this.naples.length) * 100;
      this.unknownPercent = (this.unknownNaplesCount / this.naples.length) * 100;
      this.unhealthyPercent = 100 - this.healthyPercent - this.unknownPercent;
      this.generatePieChartText();
      this.generateDoughnut();
    } else {
      this.healthyPercent = null;
      this.unhealthyPercent = null;
      this.unknownPercent = null;
    }
  }

  generatePieChartText() {
    this.pieChartPercent = Math.round(this.healthyPercent) + '%';
    this.pieChartText = 'Healthy';
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }
}
