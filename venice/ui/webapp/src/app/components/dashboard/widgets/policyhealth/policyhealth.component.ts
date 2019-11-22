import { Component, Input, OnInit, ViewEncapsulation, ViewChild } from '@angular/core';
import { OnChanges, AfterViewInit, OnDestroy } from '@angular/core/src/metadata/lifecycle_hooks';
import { Router } from '@angular/router';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { LineGraphStat, LinegraphComponent } from '@app/components/shared/linegraph/linegraph.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ChartOptions } from 'chart.js';
import { StatArrowDirection, CardStates, Stat } from '@app/components/shared/basecard/basecard.component';
import { FlipState, FlipComponent } from '@app/components/shared/flip/flip.component';
import { TelemetryPollingMetricQueries, MetricsPollingQuery, MetricsqueryService, MetricsPollingOptions } from '@app/services/metricsquery.service';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { ITelemetry_queryMetricsQueryResponse, ITelemetry_queryMetricsQueryResult } from '@sdk/v1/models/telemetry_query';
import { Telemetry_queryMetricsQuerySpec } from '@sdk/v1/models/generated/telemetry_query';

@Component({
  selector: 'app-dsbdpolicyhealth',
  templateUrl: './policyhealth.component.html',
  styleUrls: ['./policyhealth.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class PolicyhealthComponent implements OnInit, OnChanges, AfterViewInit, OnDestroy {
  @ViewChild('lineGraph') lineGraphComponent: LinegraphComponent;
  hasHover: boolean = false;
  cardStates = CardStates;

  title: string = 'Active Sessions';  // VS-645 change from 'active flows' to 'active sessions'
  firstStat: Stat = {
    value: '90%',
    description: 'ACTIVE SESSIONS',
    arrowDirection: StatArrowDirection.UP,
    statColor: '#61b3a0'
  };
  secondStat: Stat = {
    value: '5%',
    description: 'DENIED',
    arrowDirection: StatArrowDirection.UP,
    statColor: '#97b8df'
  };
  thirdStat: Stat = {
    value: '5%',
    description: 'REJECTED',
    arrowDirection: StatArrowDirection.UP,
    statColor: '#97b8df'
  };


  activeFlows: LineGraphStat = {
    title: 'ACTIVE SESSIONS',
    data: [],
    statColor: '#61b3a0',
    gradientStart: 'rgba(97, 179, 160, 1)',
    gradientStop: 'rgba(97, 179, 160, 0)',
    graphId: 'dsbdpolicyhealth-activeFlows',
    defaultValue: 0,
    defaultDescription: 'Avg',
    hoverDescription: 'Flows',
    isPercentage: false,
    scaleMin: 0,
  };

  totalConnections: LineGraphStat = {
    title: 'TOTAL CONNECTIONS PER SECOND',
    data: [],
    statColor: '#97b8df',
    gradientStart: 'rgba(151, 184, 223, 1)',
    gradientStop: 'rgba(151, 184, 223, 0)',
    graphId: 'dsbdpolicyhealth-deniedFlows',
    defaultValue: 0,
    defaultDescription: 'Avg',
    hoverDescription: 'CPS',
    isPercentage: false,
    scaleMin: 0,
  };

  linegraphStats: LineGraphStat[] = [
    this.activeFlows,
    // Removing CPS for now as it is an unreliable stat
    // this.totalConnections,
  ];

  themeColor: string = '#61b3a0';
  backgroundIcon: Icon = {
    svgIcon: 'security',
    margin: {}
  };
  icon: Icon = {
    margin: {
      top: '10px',
      left: '10px'
    },
    svgIcon: 'security'
  };
  @Input() lastUpdateTime: string;
  @Input() timeRange: string;
  // When set to true, card contents will fade into view
  cardState: CardStates = CardStates.LOADING;

  flipState: FlipState = FlipState.front;


  menuItems = [
    // {
    //   text: 'Flip card', onClick: () => {
    //     this.toggleFlip();
    //   }
    // },
  ];


  graphDrawn: boolean = false;
  viewInitialized: boolean = false;


  dataset = [];
  activePercent = 90;
  rejectPercent = 5;
  denyPercent = 5;

  dataDoughnut = {
    labels: ['active', 'rejected', 'denied'],
    datasets: [
      {
        data: [this.activePercent, this.rejectPercent, this.denyPercent],
        backgroundColor: [
          '#61b3a0',
          '#97b8df',
          'rgba 151 184 223, 0.5'
        ],
        /* hoverBackgroundColor: [
          '#a3cbf6',
          '#36A2EB'
        ] */
      }
    ]
  };

  options: ChartOptions = {
    tooltips: {
      enabled: true,
      displayColors: false,
      titleFontFamily: 'Fira Sans Condensed',
      titleFontSize: 14,
      bodyFontFamily: 'Fira Sans Condensed',
      bodyFontSize: 14,
      callbacks: {
        label: function(tooltipItem, data) {
          const dataset = data.datasets[tooltipItem.datasetIndex];
          const label = data.labels[tooltipItem.index];
          const val = dataset.data[tooltipItem.index];
          return val + '% ' + label + ' flows';
        }
      }
    },
    title: {
      display: false
    },
    legend: {
      display: false
    },
    cutoutPercentage: 0,
    circumference: 2 * Math.PI,
    rotation: (1.0 + this.rejectPercent / 100) * Math.PI, // work

    plugins: {
    },
    animation: {
      duration: 0,
    }
  };
  subscriptions = [];

  sessionData: ITelemetry_queryMetricsQueryResult;
  cpsData: ITelemetry_queryMetricsQueryResult;
  sessionAvg: ITelemetry_queryMetricsQueryResult;
  cpsAvg: ITelemetry_queryMetricsQueryResult;
  sessionDataCurrent: ITelemetry_queryMetricsQueryResult;
  currActiveFlows: number;

  constructor(private router: Router,
              protected metricsqueryService: MetricsqueryService) { }

  toggleFlip() {
    // this.flipState = FlipComponent.toggleState(this.flipState);
  }

  ngOnChanges(changes) {
  }

  ngOnInit() {
    this.getMetrics();
  }

  ngAfterViewInit() {
    this.viewInitialized = true;
    this.tryGenMetrics();
  }

  tryGenMetrics() {
    if (this.viewInitialized && this.cardState === CardStates.READY) {
      this.setupCard();
    }
  }

  getMetrics() {
    const queryList: TelemetryPollingMetricQueries = {
      queries: [],
      tenant: Utility.getInstance().getTenant()
    };
    queryList.queries.push(this.sessionTimeSeriesQuery());
    queryList.queries.push(this.cpsTimeSeriesQuery());
    queryList.queries.push(this.sessionAvgQuery());
    queryList.queries.push(this.cpsAvgQuery());
    queryList.queries.push(this.sessionCurrentQuery());

    const sub = this.metricsqueryService.pollMetrics('policyHealthCards', queryList).subscribe(
      (data: ITelemetry_queryMetricsQueryResponse) => {
        if (data && data.results && data.results.length === queryList.queries.length) {
          if (MetricsUtility.resultHasData(data.results[0])) {
            this.sessionData = data.results[0];
            this.cpsData = data.results[1];
            this.sessionAvg = data.results[2];
            this.cpsAvg = data.results[3];
            this.sessionDataCurrent = data.results[4];
            this.lastUpdateTime = new Date().toISOString();
            this.cardState = CardStates.READY;
            this.tryGenMetrics();
          } else {
            this.cardState = CardStates.READY;
            this.tryGenMetrics();
          }
        }
      },
      (err) => {
        this.cardState = CardStates.FAILED;
      }
    );
    this.subscriptions.push(sub);
  }

  setupCard() {
    if (MetricsUtility.resultHasData(this.sessionData)) {
      let data = MetricsUtility.transformToChartjsTimeSeries(this.sessionData.series[0], 'totalActiveSessions', true);
      if (MetricsUtility.resultHasData(this.sessionDataCurrent)) {
        const currData = MetricsUtility.transformToChartjsTimeSeries(this.sessionDataCurrent.series[0], 'totalActiveSessions', true);
        data = data.concat(currData);
      }
      this.activeFlows.data = data;
    }

    if (MetricsUtility.resultHasData(this.sessionDataCurrent)) {
      const currData = MetricsUtility.transformToChartjsTimeSeries(this.sessionDataCurrent.series[0], 'totalActiveSessions', true);
      this.currActiveFlows = currData[currData.length - 1].y;
    } else {
      this.currActiveFlows = 0;
    }

    if (MetricsUtility.resultHasData(this.cpsData)) {
      const data = MetricsUtility.transformToChartjsTimeSeries(this.cpsData.series[0], 'connectionsPerSecond');
      this.totalConnections.data = data;
    }

    if (MetricsUtility.resultHasData(this.sessionAvg)) {
      const index = MetricsUtility.findFieldIndex(this.sessionAvg.series[0].columns, 'totalActiveSessions');
      this.activeFlows.defaultValue = Math.round(this.sessionAvg.series[0].values[0][index]);
    }

    if (MetricsUtility.resultHasData(this.cpsAvg)) {
      const index = MetricsUtility.findFieldIndex(this.cpsAvg.series[0].columns, 'connectionsPerSecond');
      this.totalConnections.defaultValue = Math.round(this.cpsAvg.series[0].values[0][index]);
    }

    this.cardState = CardStates.READY;
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

  sessionTimeSeriesQuery(): MetricsPollingQuery {
    return MetricsUtility.timeSeriesQueryPolling('SessionSummaryMetrics', []);
  }

  sessionCurrentQuery(): MetricsPollingQuery {
    return MetricsUtility.currentFiveMinPolling('SessionSummaryMetrics');
  }

  sessionAvgQuery(): MetricsPollingQuery {
    const query: Telemetry_queryMetricsQuerySpec = MetricsUtility.pastDayAverageQuery('SessionSummaryMetrics');
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.pastDayAverageQueryUpdate,
    };
    return { query: query, pollingOptions: pollOptions };

  }

  cpsTimeSeriesQuery(): MetricsPollingQuery {
    return MetricsUtility.timeSeriesQueryPolling('FteCPSMetrics', []);
  }

  cpsAvgQuery(): MetricsPollingQuery {
    const query: Telemetry_queryMetricsQuerySpec = MetricsUtility.pastDayAverageQuery('FteCPSMetrics');
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.pastDayAverageQueryUpdate,
    };
    return { query: query, pollingOptions: pollOptions };
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

}
