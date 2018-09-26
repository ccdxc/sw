import { Component, OnDestroy, OnInit } from '@angular/core';
import { BaseComponent } from '@app/components/base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';

@Component({
  selector: 'app-settings-group',
  templateUrl: './settings-group.component.html',
  styleUrls: ['./settings-group.component.scss']
})

export class SettingsGroupComponent extends BaseComponent implements OnInit, OnDestroy {

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
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'SettingsGroupComponent', 'state': Eventtypes.COMPONENT_DESTROY });
  }
}
