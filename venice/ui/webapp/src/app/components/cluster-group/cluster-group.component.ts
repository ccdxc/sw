import { Component, OnDestroy, OnInit } from '@angular/core';
import { BaseComponent } from '@app/components/base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';

@Component({
  selector: 'app-cluster-group',
  templateUrl: './cluster-group.component.html',
  styleUrls: ['./cluster-group.component.scss']
})

export class ClusterGroupComponent implements OnInit, OnDestroy {

  constructor(protected _controllerService: ControllerService) {
  }

  ngOnInit() {
  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that this component is about to exit
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'ClusterGroupComponent', 'state': Eventtypes.COMPONENT_DESTROY });
  }
}
