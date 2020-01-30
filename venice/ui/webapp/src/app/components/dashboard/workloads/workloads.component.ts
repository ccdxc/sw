import { Component, Input, OnInit, ViewEncapsulation } from '@angular/core';
import { OnChanges, OnDestroy } from '@angular/core/src/metadata/lifecycle_hooks';
import { Router } from '@angular/router';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { LineGraphStat } from '@app/components/shared/linegraph/linegraph.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { StatArrowDirection, CardStates, Stat } from '@app/components/shared/basecard/basecard.component';
import { FlipState, FlipComponent } from '@app/components/shared/flip/flip.component';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { WorkloadWorkload, IWorkloadWorkload, IWorkloadWorkloadList } from '@sdk/v1/models/generated/workload';
import { WorkloadService } from '@app/services/generated/workload.service';
import { Subscription } from 'rxjs';
import { ClusterHost, IClusterHostList } from '@sdk/v1/models/generated/cluster';
import { ClusterService } from '@app/services/generated/cluster.service';

@Component({
  selector: 'app-dsbdworkloads',
  templateUrl: './workloads.component.html',
  styleUrls: ['./workloads.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class WorkloadsComponent implements OnInit, OnChanges, OnDestroy {
  hasHover: boolean = false;
  cardStates = CardStates;

  title: string = 'Workloads';
  firstStat: Stat = {
    value: '0',
    description: 'TOTAL WORKLOADS',
    arrowDirection: StatArrowDirection.UP,
    statColor: '#8c94ff'
  };
  secondStat: Stat = {
    value: '0',
    description: 'TOTAL HOSTS',
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

  // Used for processing watch events
  workloadEventUtility: HttpEventUtility<WorkloadWorkload>;
  workloads: ReadonlyArray<WorkloadWorkload>;
  getWorkloadsFailed: boolean = false;
  hostsEventUtility: HttpEventUtility<ClusterHost>;
  hosts: ReadonlyArray<ClusterHost>;
  getHostsFailed: boolean = false;

  avgWorkloadPerHost: number = 0;

  @Input() lastUpdateTime: string;
  @Input() timeRange: string;
  // When set to true, card contents will fade into view
  @Input() cardState: CardStates = CardStates.LOADING;

  flipState: FlipState = FlipState.front;

  subscriptions: Subscription[] = [];

  menuItems = [
    // {
    //   text: 'Flip card', onClick: () => {
    //     this.toggleFlip();
    //   }
    // },
    {
      text: 'Navigate to Workload', onClick: () => {
        this.goToWorkloads();
      }
    }
  ];

  showGraph: boolean = false;

  constructor(private router: Router,
              protected workloadService: WorkloadService,
              protected clusterService: ClusterService) { }

  toggleFlip() {
    // Card is currently not allowed to be flipped
    // this.flipState = FlipComponent.toggleState(this.flipState);
  }

  ngOnChanges(changes) {
  }

  goToWorkloads() {
    this.router.navigateByUrl('/workload');
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
    this.getWorkloads();
    this.getHosts();
  }

  getWorkloads() {
    const subscription = this.workloadService.ListWorkloadCache().subscribe(
      (response) => {
        this.workloads = response;
        this.firstStat.value = this.workloads.length.toString();
        this.cardState = CardStates.READY;
        this.computeAvg();
      },
      (err) => {
        this.getWorkloadsFailed = true;
        this.checkFailureState();
      }
    );
    this.subscriptions.push(subscription);
  }

  getHosts() {
    const subscription = this.clusterService.ListHostCache().subscribe(
      response => {
        this.hosts = response;
        this.secondStat.value = this.hosts.length.toString();
        this.cardState = CardStates.READY;
        this.computeAvg();
      },
      (err) => {
        this.getHostsFailed = true;
        this.checkFailureState();
      }
    );
    this.subscriptions.push(subscription);
  }

  checkFailureState() {
    if (this.getWorkloadsFailed && this.getHostsFailed) {
      this.cardState = CardStates.FAILED;
    }
  }

  computeAvg() {
    if (this.hosts && this.hosts.length === 0) {
      this.avgWorkloadPerHost = 0;
    } else if (this.workloads && this.hosts) {
      this.avgWorkloadPerHost = Math.round(this.workloads.length / this.hosts.length);
    }
  }

  ngOnDestroy() {
    if (this.subscriptions != null) {
      this.subscriptions.forEach(subscription => {
        subscription.unsubscribe();
      });
    }
  }

}
