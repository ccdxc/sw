import { Component, OnDestroy, OnInit, ViewEncapsulation} from '@angular/core';
import { ControllerService } from '../../services/controller.service';
import { CommonComponent } from '../../common.component';
import { Subscription } from 'rxjs/Subscription';

import { Eventtypes } from '@app/enum/eventtypes.enum';

@Component({
  selector: 'app-toolbar',
  templateUrl: './toolbar.component.html',
  styleUrls: ['./toolbar.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class ToolbarComponent extends CommonComponent implements OnInit, OnDestroy {
  private subscription: Subscription;
  breadcrumb: any;
  buttons: any;

  constructor(protected _controllerService: ControllerService) {
    super();
  }

  ngOnInit() {
    this.breadcrumb = [{label: ''}];
    this.subscription = this._controllerService.subscribe(Eventtypes.TOOLBAR_DATA_CHANGE,
      (data) => {
        this.breadcrumb = data.breadcrumb;
        this.buttons = data.buttons;
      });
  }

  ngOnDestroy() {
    this.subscription.unsubscribe();
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'ToolbarComponent', 'state': Eventtypes.COMPONENT_DESTROY });
  }

  buttonCallback(event, button) {
    button.callback(event);
  }
}
