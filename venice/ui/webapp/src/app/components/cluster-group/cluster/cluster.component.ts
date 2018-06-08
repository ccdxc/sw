import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/cluster.service';
import { BaseComponent } from '../../base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';

@Component({
  selector: 'app-cluster',
  encapsulation: ViewEncapsulation.None,
  templateUrl: './cluster.component.html',
  styleUrls: ['./cluster.component.scss']
})
export class ClusterComponent extends BaseComponent implements OnInit {

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/cluster/ico-cluster-black.svg'
  };
  cluster: any;

  constructor(
    private _clusterService: ClusterService,
    protected _controllerService: ControllerService,
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    if (!this._controllerService.isUserLogin()) {
      this._controllerService.publish(Eventtypes.NOT_YET_LOGIN, {});
    } else {
      this._clusterService.getCluster().subscribe(
        data => {
          if (data.Items.length > 0) {
            this.cluster = data.Items[0]
          }
        }
      );
    }
  }
}
