import { Component, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { AlertseventsComponent } from '@app/components/shared/alertsevents/alertsevents.component';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { IApiStatus, ClusterCluster, ClusterNode } from '@sdk/v1/models/generated/cluster';
import { Table } from 'primeng/table';
import { Subscription } from 'rxjs/Subscription';
import { BaseComponent } from '@app/components/base/base.component';
import { MessageService } from 'primeng/primeng';

@Component({
  selector: 'app-cluster',
  encapsulation: ViewEncapsulation.None,
  templateUrl: './cluster.component.html',
  styleUrls: ['./cluster.component.scss']
})
export class ClusterComponent extends BaseComponent implements OnInit, OnDestroy {
  @ViewChild('nodestable') nodesTable: Table;
  @ViewChild(AlertseventsComponent) alertsEventsComponent: AlertseventsComponent;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/cluster/ico-cluster-black.svg'
  };
  cluster: ClusterCluster;
  // Used for processing the stream events
  clusterEventUtility: HttpEventUtility<ClusterCluster>;
  nodeEventUtility: HttpEventUtility<ClusterNode>;

  clusterArray: ReadonlyArray<ClusterCluster> = [];
  nodes: ReadonlyArray<ClusterNode> = [];

  cols: any[] = [
    { field: 'name', header: 'Name' },
    { field: 'quorum', header: 'Quorum Member' },
    { field: 'phase', header: 'Phase' },
  ];
  subscriptions: Subscription[] = [];

  constructor(
    private _clusterService: ClusterService,
    protected _controllerService: ControllerService,
    protected messageService: MessageService
  ) {
    super(_controllerService, messageService);
  }

  ngOnInit() {
    this.getCluster();
    this.getNodes();

    this._controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [{ label: 'Cluster', url: '' }, { label: 'Cluster', url: '' }]
    });
  }

  getCluster() {
    this.clusterEventUtility = new HttpEventUtility<ClusterCluster>(ClusterCluster, true);
    this.clusterArray = this.clusterEventUtility.array as ReadonlyArray<ClusterCluster>;
    const subscription = this._clusterService.WatchCluster().subscribe(
      response => {
        const body: any = response.body;
        this.clusterEventUtility.processEvents(body);
        if (this.clusterArray.length > 0) {
          this.cluster = this.clusterArray[0];
        }
      },
      this.restErrorHandler('Failed to get Cluster info')
    );
    this.subscriptions.push(subscription);
  }

  getNodes() {
    this.nodeEventUtility = new HttpEventUtility<ClusterNode>(ClusterNode);
    this.nodes = this.nodeEventUtility.array;
    const subscription = this._clusterService.WatchNode().subscribe(
      response => {
        const body: any = response.body;
        this.nodeEventUtility.processEvents(body);
      },
      this.restErrorHandler('Failed to get Node info')
    );
    this.subscriptions.push(subscription);
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }
}
