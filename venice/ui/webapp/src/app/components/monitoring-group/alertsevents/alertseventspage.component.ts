import { Component, OnInit, ViewEncapsulation, OnDestroy } from '@angular/core';
import { BaseComponent } from '@app/components/base/base.component';
import { ControllerService } from '@app/services/controller.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { MessageService } from 'primeng/primeng';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Utility } from '@app/common/Utility';

@Component({
  selector: 'app-alertsevents',
  templateUrl: './alertseventspage.component.html',
  styleUrls: ['./alertseventspage.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class AlertseventspageComponent extends BaseComponent implements OnInit, OnDestroy {
  hideAlertpolicies: boolean = this.uiconfigsService.isObjectDisabled('AlertPolicies');

  breadCrumb = [{ label: 'Alerts & Events', url: Utility.getBaseUIUrl() + 'monitoring/alertsevents' }];


  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected messageService: MessageService) {
    super(_controllerService, messageService);
  }

  ngOnInit() {
    // Default tab is alerts
    this.setTabToolbar('alerts');
  }

  setTabToolbar(tabName) {
    let buttons = [];
    if (tabName === 'alerts') {
      buttons = [
        {
          cssClass: 'global-button-primary alertsevents-button',
          text: 'ALERT POLICIES',
          callback: () => { this._controllerService.navigate(['/monitoring', 'alertsevents', 'alertpolicies']); }
        },
      ];
    } else {
      buttons = [
        {
          cssClass: 'global-button-primary alertsevents-button',
          text: 'EVENT POLICY',
          callback: () => { this._controllerService.navigate(['/monitoring', 'alertsevents', 'eventpolicy']); }
        }
      ];
    }
    this._controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: this.breadCrumb
    });
  }

  getClassName(): string {
    return this.constructor.name;
  }

  ngOnDestroy() {
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, {
      'component': 'Alertseventspage', 'state':
        Eventtypes.COMPONENT_DESTROY
    });
  }
}
