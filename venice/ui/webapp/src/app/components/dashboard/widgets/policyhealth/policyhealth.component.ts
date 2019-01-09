import { Component, Input, OnInit, ViewEncapsulation } from '@angular/core';
import { OnChanges } from '@angular/core/src/metadata/lifecycle_hooks';
import { Router } from '@angular/router';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { LineGraphStat, LinegraphComponent } from '@app/components/shared/linegraph/linegraph.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ChartOptions } from 'chart.js';
import { StatArrowDirection, CardStates, Stat } from '@app/components/shared/basecard/basecard.component';
import { FlipState, FlipComponent } from '@app/components/shared/flip/flip.component';

@Component({
  selector: 'app-dsbdpolicyhealth',
  templateUrl: './policyhealth.component.html',
  styleUrls: ['./policyhealth.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class PolicyhealthComponent implements OnInit, OnChanges {
  hasHover: boolean = false;
  cardStates = CardStates;

  title: string = 'Security Policies';
  firstStat: Stat = {
    value: '90%',
    description: 'ACTIVE FLOWS',
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
    title: 'ACTIVE FLOWS',
    data: [],
    statColor: '#61b3a0',
    gradientStart: 'rgba(97, 179, 160, 1)',
    gradientStop: 'rgba(97, 179, 160, 0)',
    graphId: 'dsbdpolicyhealth-activeFlows',
    defaultValue: 10,
    defaultDescription: 'Avg',
    hoverDescription: '',
    isPercentage: false,
    valueFormatter: LinegraphComponent.percentFormatter
  };

  deniedFlows: LineGraphStat = {
    title: 'DENIED FLOWS',
    data: [],
    statColor: '#97b8df',
    gradientStart: 'rgba(151, 184, 223, 1)',
    gradientStop: 'rgba(151, 184, 223, 0)',
    graphId: 'dsbdpolicyhealth-deniedFlows',
    defaultValue: 10,
    defaultDescription: 'Avg',
    hoverDescription: '',
    isPercentage: false,
    valueFormatter: LinegraphComponent.percentFormatter
  };

  rejectedFlows: LineGraphStat = {
    title: 'REJECTED FLOWS',
    data: [],
    statColor: '#97b8df',
    gradientStart: 'rgba(151, 184, 223, 1)',
    gradientStop: 'rgba(151, 184, 223, 0)',
    graphId: 'dsbdpolicyhealth-rejectedFlows',
    defaultValue: 10,
    defaultDescription: 'Avg',
    hoverDescription: '',
    isPercentage: false,
    valueFormatter: LinegraphComponent.percentFormatter
  };

  linegraphStats: LineGraphStat[] = [
    this.activeFlows,
    this.deniedFlows,
    this.rejectedFlows
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

  constructor(private router: Router) { }

  toggleFlip() {
    this.flipState = FlipComponent.toggleState(this.flipState);
  }

  ngOnChanges(changes) {
  }

  ngOnInit() {
    const chartData = [this.activeFlows, this.rejectedFlows, this.deniedFlows];
    chartData.forEach((chart) => {
      const data = [];
      const oneDayAgo = new Date(new Date().getTime() - (24 * 60 * 60 * 1000));
      for (let index = 0; index < 48; index++) {
        data.push({ t: new Date(oneDayAgo.getTime() + (index * 30 * 60 * 1000)), y: Utility.getRandomInt(0, 20) });
      }
      chart.data = data;
    });
  }

}
