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
import { Telemetry_queryMetricsQuerySpec, Telemetry_queryMetricsQuerySpec_function } from '@sdk/v1/models/generated/telemetry_query';
/**
SessionSummaryMetrics table looks like this

time	dsc	TotalActiveSession
t0-1	dsc-1	1
t0-1	dsc-2	2
t0-1	dsc-3	3
t0-1	dsc-4	4
t0-1	dsc-5	5	 15	t0-1 window all DSCs total sessions. avg 15/5=3
t0-2	dsc-1	11
t0-2	dsc-2	12
t0-2	dsc-3	13
t0-2	dsc-4	14
t0-2	dsc-5	15	65
t1-1	dsc-1	21
t1-1	dsc-2	22
t1-1	dsc-3	23
t1-1	dsc-4	24
t1-1	dsc-5	25	115
t1-2	dsc-1	31
t1-2	dsc-2	32
t1-2	dsc-3	33
t1-2	dsc-4	34
t1-2	dsc-5	35	165	current active sessions (dsc1-dsc5)

Where from t0-1 to t0-2, there are 30 seconds time gap.
===
In t0 window, dsc-1 total sessions is 1+11 = 12, averge is 6. However, (t0-2	dsc-1	11) may contain the session in (t0-1	dsc-1	1)

 */
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
    hoverDescription: 'Sessions',
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
        label: function (tooltipItem, data) {
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
    queryList.queries.push(this.sessionTimeSeriesQuery()); // time series. Every 5 minutes last Sum(all DSC TotalActiveSessions)
    queryList.queries.push(this.cpsTimeSeriesQuery());
    queryList.queries.push(this.sessionAvgQuery());
    queryList.queries.push(this.cpsAvgQuery());
    queryList.queries.push(this.sessionCurrentQuery());  // current TotalActiveSessions of all DSCs

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
      if (data.length > 1) {
        const start = data[0].t;
        const end = data[data.length - 1].t;
        const moment = Utility.getMomentJS();
        const startUtc = moment(start).utc();
        const endUtc = moment(end).utc();
        const startDay = startUtc.dayOfYear();
        const endDay = endUtc.dayOfYear();
        const formatOpts = startDay !== endDay ? 'ddd HH:mm z' : 'HH:mm z';
        const startStr = startUtc.format(formatOpts);
        const endStr = endUtc.format(formatOpts);

        this.activeFlows.title = `ACTIVE SESSIONS (${startStr} - ${endStr})`;
      } else {
        this.activeFlows.title = 'ACTIVE SESSIONS';
      }
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

  /**
   Every 5 minutes window, take 5-minutes-window's mean(all DSC average TotalActiveSessions)
   {
      "tenant": "default",
      "namespace": null,
      "queries": [
        {
          "kind": "SessionSummaryMetrics",
          "api-version": null,
          "name": null,
          "function": "mean",
          "start-time": "2019-12-05T19:00:25.195Z",
          "end-time": "now()",
          "group-by-time": "5m",
          "sort-order": "ascending"
        }
      ]
    }
   */
  sessionTimeSeriesQuery(): MetricsPollingQuery {
    return MetricsUtility.timeSeriesQueryPolling('SessionSummaryMetrics', [], null, Telemetry_queryMetricsQuerySpec_function.mean);
  }

  /**
   * 2019-12-05 Per discussion with Ranjith and Jeff
   * metrics query request JSON as below (we want to have Sum(all DSC the lastest record's TotalActiveSessions )
   {
      "tenant": "default",
      "namespace": null,
      "queries": [
        {
          "kind": "SessionSummaryMetrics",
          "api-version": null,
          "name": null,
          "function": "max",
          "start-time": "2019-12-05T19:12:25.195Z",
          "end-time": "now()",
          "group-by-time": null,
          "group-by-field": null,
          "sort-order": "ascending"
        }
      ]
    }
   */
  /**
   TODO: // why we want to use "max" function for now 2019-12-09
   Say we have the following points, reported in the given order:

    t1-2	dsc-3	90
    t1-2	dsc-4	90
    t1-2	dsc-5	0
   the "last" function will return 0 since dsc-5 returned 0.
   The last function won't aggregate across the naples before picking a value, it will just pick the last reported point. This value isn't indicative of the actual current CPS

   Ideally, we should get have a "sum" function. This will give as 180 (90 + 90 + 0).  That is all the active sessions across all dsc-(3 to 5)
   */
  sessionCurrentQuery(): MetricsPollingQuery {
    return MetricsUtility.currentFiveMinPolling('SessionSummaryMetrics', null, ['TotalActiveSessions'],  Telemetry_queryMetricsQuerySpec_function.max);
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
