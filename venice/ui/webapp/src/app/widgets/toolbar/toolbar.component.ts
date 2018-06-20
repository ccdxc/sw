import { Component, OnDestroy, OnInit, ViewEncapsulation, TemplateRef } from '@angular/core';
import { ControllerService } from '../../services/controller.service';
import { CommonComponent } from '../../common.component';
import { Subscription } from 'rxjs/Subscription';

import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ToolbarButton, BreadcrumbItem, ToolbarData } from '@app/models/frontend/shared/toolbar.interface';
import { Utility } from '@app/common/Utility';

@Component({
  selector: 'app-toolbar',
  templateUrl: './toolbar.component.html',
  styleUrls: ['./toolbar.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class ToolbarComponent extends CommonComponent implements OnInit, OnDestroy {
  private subscription: Subscription;
  breadcrumb: BreadcrumbItem[] = [{ label: '' }];
  buttons: ToolbarButton[] = [];

  constructor(protected _controllerService: ControllerService) {
    super();
  }

  ngOnInit() {
    this.subscription = this._controllerService.subscribe(Eventtypes.TOOLBAR_DATA_CHANGE,
      (data: ToolbarData) => {
        data = Utility.getLodash().cloneDeep(data);
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

  public clear() {
    if (this.breadcrumb) {
      this.breadcrumb.length = 0;
    }
    if (this.buttons) {
      this.buttons.length = 0;
    }
  }
}
