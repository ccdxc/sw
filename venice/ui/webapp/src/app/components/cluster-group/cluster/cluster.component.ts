import { Component, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { BaseComponent } from '../../base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Table } from 'primeng/table';
import { IApiStatus, ClusterCluster, IClusterClusterList, ClusterClusterList, ClusterNode, IClusterNodeList, ClusterNodeList } from '@sdk/v1/models/generated/cluster';

@Component({
  selector: 'app-cluster',
  encapsulation: ViewEncapsulation.None,
  templateUrl: './cluster.component.html',
  styleUrls: ['./cluster.component.scss']
})
export class ClusterComponent extends BaseComponent implements OnInit {
  @ViewChild('nodestable') nodesTable: Table;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/cluster/ico-cluster-black.svg'
  };
  cluster: ClusterCluster;
  nodes: ClusterNode[] = [];
  nodeCount: Number = 0;
  cols: any[] = [
    { field: 'name', header: 'Name' },
    { field: 'quorum', header: 'Quorum Member' },
    { field: 'phase', header: 'Phase' },
  ];

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
      buttons: [
        {
          cssClass: 'global-button-primary cluster-toolbar-button',
          text: 'Refresh',
          callback: () => { this.getCluster(); this.getNodes(); },
        }],
      breadcrumb: [{ label: 'Cluster', url: '' }, { label: 'Cluster', url: '' }]
    });
  }

  getCluster() {
    this._clusterService.ListCluster().subscribe(
      (data) => {
        const clusters: ClusterClusterList = new ClusterClusterList(<IClusterClusterList>data.body);

        if (clusters.Items.length > 0) {
          this.cluster = clusters.Items[0];
        }
      },
      (error) => {
        // TODO: Error handling
        if (error.body instanceof Error) {
          console.log('Cluster service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
        } else {
          console.log('Cluster service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
        }
      }
    );
  }

  getNodes() {
    this._clusterService.ListNode().subscribe(
      data => {
        if (data.statusCode !== 200) {
          console.log('Node service returned code: ' + data.statusCode + ' data: ' + <IApiStatus>data.body);
          // TODO: Error handling
          return;
        }
        const nodes: ClusterNodeList = new ClusterNodeList(<IClusterNodeList>data.body);

        this.nodeCount = nodes.Items.length;
        this.nodes = nodes.Items;
      }
    );
  }
}
