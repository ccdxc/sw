import { Component, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { AlertseventsComponent } from '@app/components/shared/alertsevents/alertsevents.component';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { IApiStatus, IClusterCluster, IClusterNode } from '@sdk/v1/models/generated/cluster';
import { Table } from 'primeng/table';
import { Subscription } from 'rxjs/Subscription';
import { BaseComponent } from '@app/components/base/base.component';

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
  cluster: IClusterCluster;
  // Used for processing the stream events
  clusterEventUtility: HttpEventUtility;
  nodeEventUtility: HttpEventUtility;

  clusterArray: ReadonlyArray<IClusterCluster> = [];
  nodes: ReadonlyArray<IClusterNode> = [];

  cols: any[] = [
    { field: 'name', header: 'Name' },
    { field: 'quorum', header: 'Quorum Member' },
    { field: 'phase', header: 'Phase' },
  ];
  subscriptions: Subscription[] = [];

  constructor(
    private _clusterService: ClusterService,
    protected _controllerService: ControllerService,
  ) {
    super(_controllerService);
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
    this.clusterEventUtility = new HttpEventUtility();
    this.clusterArray = this.clusterEventUtility.array as ReadonlyArray<IClusterCluster>;
    const subscription = this._clusterService.WatchCluster().subscribe(
      response => {
        const body: any = response.body;
        this.clusterEventUtility.processEvents(body);
        if (this.clusterArray.length > 0) {
          this.cluster = this.clusterArray[0];
        }
      },
      error => {
        // TODO: Error handling
        if (error.body instanceof Error) {
          console.log('Cluster service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
        } else {
          console.log('Cluster service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
        }
      }
    )
    this.subscriptions.push(subscription);
  }

  getNodes() {
    this.nodeEventUtility = new HttpEventUtility();
    this.nodes = this.nodeEventUtility.array;
    const subscription = this._clusterService.WatchNode().subscribe(
      response => {
        const body: any = response.body;
        this.nodeEventUtility.processEvents(body);
      },
      error => {
        // TODO: Error handling
        if (error.body instanceof Error) {
          console.log('Cluster service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
        } else {
          console.log('Cluster service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
        }
      }
    )
    this.subscriptions.push(subscription);
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }
}
