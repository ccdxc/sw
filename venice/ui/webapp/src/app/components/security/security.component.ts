import { Component, OnDestroy, OnInit } from '@angular/core';
import { BaseComponent } from '@app/components/base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';



@Component({
  selector: 'app-security',
  templateUrl: './security.component.html',
  styleUrls: ['./security.component.scss']
})
export class SecurityComponent extends BaseComponent implements OnInit, OnDestroy {

  constructor(protected _controllerService: ControllerService,
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    if (!this._controllerService.isUserLogin()) {
      this._controllerService.publish(Eventtypes.NOT_YET_LOGIN, {});
    } else {

    }
  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to exist
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'sgpolicyComponent', 'state': Eventtypes.COMPONENT_DESTROY });
  }

}
