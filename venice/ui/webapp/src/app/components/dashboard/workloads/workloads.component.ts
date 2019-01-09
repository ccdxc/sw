import { Component, Input, OnInit, ViewEncapsulation } from '@angular/core';
import { OnChanges } from '@angular/core/src/metadata/lifecycle_hooks';
import { Router } from '@angular/router';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { LineGraphStat } from '@app/components/shared/linegraph/linegraph.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { StatArrowDirection, CardStates, Stat } from '@app/components/shared/basecard/basecard.component';
import { FlipState, FlipComponent } from '@app/components/shared/flip/flip.component';

@Component({
  selector: 'app-dsbdworkloads',
  templateUrl: './workloads.component.html',
  styleUrls: ['./workloads.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class WorkloadsComponent implements OnInit, OnChanges {
  hasHover: boolean = false;
  cardStates = CardStates;

  title: string = 'Workloads';
  firstStat: Stat = {
    value: '1,244',
    description: 'TOTAL WORKLOADS',
    arrowDirection: StatArrowDirection.UP,
    statColor: '#8c94ff'
  };
  secondStat: Stat = {
    value: '30 KB',
    description: 'AVG TRAFFIC PER WORKLOAD',
    arrowDirection: StatArrowDirection.UP,
    statColor: '#8c94ff'
  };

  totalWorkloadsStat: LineGraphStat = {
    title: 'TOTAL WORKLOADS',
    data: [],
    statColor: '#8c94ff',
    gradientStart: 'rgba(140,148,255, 1)',
    gradientStop: 'rgba(140,148,255, 0)',
    graphId: 'dsbdworkloads-totalWorkloads',
    defaultValue: 600,
    defaultDescription: 'Avg',
    hoverDescription: 'Workloads',
    isPercentage: false
  };
  avgTrafficStat: LineGraphStat = {
    title: 'AVERAGE TRAFFIC PER WORKLOAD',
    data: [],
    statColor: '#8c94ff',
    gradientStart: 'rgba(140,148,255, 1)',
    gradientStop: 'rgba(140,148,255, 0)',
    graphId: 'dsbdworkloads-trafficWorkloads',
    defaultValue: 10,
    defaultDescription: 'Avg',
    hoverDescription: 'KB',
    isPercentage: false
  };

  linegraphStats: LineGraphStat[] = [
    this.totalWorkloadsStat,
    this.avgTrafficStat,
  ];

  themeColor: string = '#8c94ff';
  backgroundIcon: Icon = {
    svgIcon: 'workloads',
    margin: {}
  };
  icon: Icon = {
    margin: {
      top: '10px',
      left: '10px'
    },
    svgIcon: 'workloads'
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

  constructor(private router: Router) { }

  toggleFlip() {
    this.flipState = FlipComponent.toggleState(this.flipState);
  }

  ngOnChanges(changes) {
  }

  ngOnInit() {
    const chartData = [this.totalWorkloadsStat, this.avgTrafficStat];
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
