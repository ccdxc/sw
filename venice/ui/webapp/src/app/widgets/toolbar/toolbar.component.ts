import { Component, OnDestroy, OnInit, ViewEncapsulation, TemplateRef } from '@angular/core';
import { ControllerService } from '../../services/controller.service';
import { CommonComponent } from '../../common.component';
import { Subscription } from 'rxjs';

import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ToolbarButton, BreadcrumbItem, ToolbarData, SplitButton, Dropdown } from '@app/models/frontend/shared/toolbar.interface';
import { Utility } from '@app/common/Utility';

/**
 * ToolbarComponent is a widget.  It shows breadcrumb and hosts buttons/split-buttons/dropdowns
 *
 *  2018-10-24
 *  See DashboardComponent and SearchresultComponent on how to use toolbar
 *
 */
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
  splitbuttons: SplitButton[] = [];
  dropdowns: Dropdown[] = [];

  constructor(protected _controllerService: ControllerService) {
    super();
  }

  ngOnInit() {
    this.subscription = this._controllerService.subscribe(Eventtypes.TOOLBAR_DATA_CHANGE,
      (data: ToolbarData) => {
        data = Utility.getLodash().cloneDeep(data);
        this.breadcrumb = data.breadcrumb;
        this.buttons = data.buttons;
        this.splitbuttons = (data.splitbuttons) ? data.splitbuttons : [];  // get split-buttons configure
        this.dropdowns = (data.dropdowns) ? data.dropdowns : []; // get dropdowns configure
      });
  }

  ngOnDestroy() {
    if (this.subscription) {
      this.subscription.unsubscribe();
    }
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'ToolbarComponent', 'state': Eventtypes.COMPONENT_DESTROY });
  }

  /**
   * Server html template, invoke pass-in function
   * @param event
   * @param button
   */
  buttonCallback(event, button) {
    button.callback(event);
  }

  /**
   * Server html template, invoke pass-in function
   * @param event
   * @param button
   */
  splitbuttonCallback(event, splitbutton) {
    splitbutton.callback(event, splitbutton);
  }

  /**
   * Server html template, invoke pass-in function
   * @param event
   * @param button
   */
  dropdownChangeCallback(event, dropdown) {
    dropdown.callback(event, dropdown);
  }

  public clear() {
    if (this.breadcrumb) {
      this.breadcrumb.length = 0;
    }
    if (this.buttons) {
      this.buttons.length = 0;
    }
    if (this.splitbuttons) {
      this.splitbuttons.length = 0;
    }
    if (this.dropdowns) {
      this.dropdowns.length = 0;
    }
  }
}
