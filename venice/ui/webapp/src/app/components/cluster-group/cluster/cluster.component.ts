import { Component, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/cluster.service';
import { NodesService } from '@app/services/nodes.service';
import { BaseComponent } from '../../base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import {Table} from 'primeng/table';

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
  cluster: any;
  nodes: any[] = [];
  nodeCount: 0;
  cols: any[] = [
    { field: 'name', header: 'Name' },
    { field: 'quorum', header: 'Quorum Member' },
    { field: 'phase', header: 'Phase' },
  ];

  constructor(
    private _clusterService: ClusterService,
    private _nodesService: NodesService,
    protected _controllerService: ControllerService,
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    if (!this._controllerService.isUserLogin()) {
      this._controllerService.publish(Eventtypes.NOT_YET_LOGIN, {});
    } else {
      this.getCluster();
      this.getNodes();

      this._controllerService.setToolbarData({
        buttons: [
          {
            cssClass: 'global-button-primary cluster-toolbar-refresh-button',
            text: 'Refresh',
            callback: () => { this.getCluster(); this.getNodes(); },
          }],
        breadcrumb: [{ label: 'Cluster', url: ''}, {label: 'Cluster', url: ''}]
      });
    }
  }

  getCluster() {
    this._clusterService.getCluster().subscribe(
      data => {
        if (data.Items.length > 0) {
          this.cluster = data.Items[0];
        }
      }
    );
  }

  getNodes() {
    this._nodesService.getNodes().subscribe(
      data => {
        this.nodeCount = data.Items.length;
        for (let i = 0; i < data.Items.length; i++) {
          if (data.Items[i].status.quorum === true) {
            data.Items[i].status.quorum = 'yes';
          }
          const phase = String(data.Items[i].status.phase);
          data.Items[i].status.phase = phase.charAt(0).toUpperCase() + phase.slice(1).toLowerCase();
        }
        this.nodes = data.Items;
      }
    );
  }
}
