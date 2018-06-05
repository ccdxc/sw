import { Component, OnInit } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/cluster.service';
import { BaseComponent } from '../../base/base.component';

@Component({
  selector: 'app-cluster',
  templateUrl: './cluster.component.html',
  styleUrls: ['./cluster.component.scss']
})
export class ClusterComponent extends BaseComponent implements OnInit {

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/cluster/ico-cluster.svg'
  };

  constructor(
    private _clusterService: ClusterService,
    protected _controllerService: ControllerService,
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    this._clusterService.getCluster().subscribe( (data) => {console.log("Data ", data);});
  }

}
