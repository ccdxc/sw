import { Component, OnDestroy, OnInit } from '@angular/core';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';

@Component({
  selector: 'app-troubleshoot-group',
  templateUrl: '../monitoring-group/monitoring-group.component.html',
  styleUrls: ['../monitoring-group/monitoring-group.component.scss']
})
export class TroubleshootGroupComponent implements OnInit, OnDestroy {

  constructor(protected _controllerService: ControllerService) {
  }

  ngOnInit() {
  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that this component is about to exit
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'TroubleshootGroupComponent', 'state': Eventtypes.COMPONENT_DESTROY });
  }
}

