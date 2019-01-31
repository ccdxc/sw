import { Component, Input, OnInit, ViewEncapsulation } from '@angular/core';
import { OnChanges, OnDestroy } from '@angular/core/src/metadata/lifecycle_hooks';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { LineGraphStat } from '@app/components/shared/linegraph/linegraph.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ChartOptions } from 'chart.js';
import { StatArrowDirection, CardStates, Stat } from '@app/components/shared/basecard/basecard.component';
import { FlipState, FlipComponent } from '@app/components/shared/flip/flip.component';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { ClusterSmartNIC, ClusterSmartNICStatus_admission_phase } from '@sdk/v1/models/generated/cluster';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ControllerService } from '@app/services/controller.service';
import { Subscription } from 'rxjs';

@Component({
  selector: 'app-dsbdnapleswidget',
  templateUrl: './naples.component.html',
  styleUrls: ['./naples.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NaplesComponent implements OnInit, OnChanges, OnDestroy {
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
    title: 'TOTAL NAPLES',
    data: [],
    statColor: '#b592e3',
    gradientStart: 'rgba(181,146, 227,1)',
    gradientStop: 'rgba(181,146, 227,0)',
    graphId: 'dsbdnaples-totalNaples',
    defaultValue: 600,
    defaultDescription: 'Avg',
    hoverDescription: 'Naples',
    isPercentage: false
  };
  rejectedNaplesStat: LineGraphStat = {
    title: 'REJECTED NAPLES',
    data: [],
    statColor: '#e57553',
    gradientStart: 'rgba(229, 117, 83, 1)',
    gradientStop: 'rgba(229, 117, 83, 0)',
    graphId: 'dsbdnaples-rejectedNaples',
    defaultValue: 10,
    defaultDescription: 'Avg',
    hoverDescription: 'Naples',
    isPercentage: false
  };
  pendingNaplesStat: LineGraphStat = {
    title: 'PENDING NAPLES',
    data: [],
    statColor: '#97b8df',
    gradientStart: 'rgba(151, 184, 223, 1)',
    gradientStop: 'rgba(151, 184, 223, 0)',
    graphId: 'dsbdnaples-pendingNaples',
    defaultValue: 10,
    defaultDescription: 'Avg',
    hoverDescription: 'Naples',
    isPercentage: false
  };

  linegraphStats: LineGraphStat[] = [
    this.totalNaplesStat,
    this.rejectedNaplesStat,
    this.pendingNaplesStat
  ];

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
  @Input() cardState: CardStates = CardStates.READY;

  flipState: FlipState = FlipState.front;


  menuItems = [
    {
      text: 'Flip card', onClick: () => {
        this.toggleFlip();
      }
    },
  ];


  showGraph: boolean = false;


  dataset = [];
  healthyPercent = 90;
  unhealthyPercent = 10;

  dataDoughnut = {
    labels: ['Healthy', 'Unhealthy'],
    datasets: [
      {
        data: [this.healthyPercent, this.unhealthyPercent],
        backgroundColor: [
          '#97b8df',
          '#e57553'
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
          if (label === 'Healthy') {
            return val + '% of Naples are healthy';
          } else {
            return val + '% of Naples have critical errors';
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

  constructor(private controllerService: ControllerService,
    protected clusterService: ClusterService) { }

  toggleFlip() {
    this.flipState = FlipComponent.toggleState(this.flipState);
  }

  ngOnChanges(changes) {
  }

  ngOnInit() {
    this.getNaples()
    const chartData = [this.totalNaplesStat, this.rejectedNaplesStat, this.pendingNaplesStat];
    chartData.forEach((chart) => {
      const data = [];
      const oneDayAgo = new Date(new Date().getTime() - (24 * 60 * 60 * 1000));
      for (let index = 0; index < 48; index++) {
        data.push({ t: new Date(oneDayAgo.getTime() + (index * 30 * 60 * 1000)), y: Utility.getRandomInt(0, 20) });
      }
      chart.data = data;
    });
  }

  getNaples() {
    this.naplesEventUtility = new HttpEventUtility<ClusterSmartNIC>(ClusterSmartNIC);
    this.naples = this.naplesEventUtility.array as ReadonlyArray<ClusterSmartNIC>;
    const subscription = this.clusterService.WatchSmartNIC().subscribe(
      response => {
        this.naplesEventUtility.processEvents(response);
        this.calculateNaplesStatus()
      },
      this.controllerService.restErrorHandler('Failed to get NAPLES info')
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  calculateNaplesStatus() {
    let rejected = 0; let admitted = 0; let pending = 0;
    this.naples.forEach((naple) => {
      switch (naple.status["admission-phase"]) {
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
    })
    this.firstStat.value = this.naples.length.toString()
    this.secondStat.value = admitted.toString();
    this.thirdStat.value = rejected.toString();
    this.fourthStat.value = pending.toString();
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

}
