import { Component, OnDestroy, OnInit } from '@angular/core';
import { BaseComponent } from '@app/components/base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';

@Component({
  selector: 'app-monitoring-group',
  templateUrl: './monitoring-group.component.html',
  styleUrls: ['./monitoring-group.component.scss']
})
export class MonitoringGroupComponent extends BaseComponent implements OnInit, OnDestroy {

  constructor(protected _controllerService: ControllerService) {
    super(_controllerService);
  }

  ngOnInit() {
  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that this component is about to exit
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'MonitoringGroupComponent', 'state': Eventtypes.COMPONENT_DESTROY });
  }
}

