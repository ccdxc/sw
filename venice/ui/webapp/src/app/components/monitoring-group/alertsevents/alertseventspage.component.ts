import { Component, OnInit, ViewEncapsulation, OnDestroy, ViewChild } from '@angular/core';
import { BaseComponent } from '@app/components/base/base.component';
import { ControllerService } from '@app/services/controller.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Utility } from '@app/common/Utility';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { AlertseventsComponent } from '@app/components/shared/alertsevents/alertsevents.component';

@Component({
  selector: 'app-alertsevents',
  templateUrl: './alertseventspage.component.html',
  styleUrls: ['./alertseventspage.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class AlertseventspageComponent extends BaseComponent implements OnInit, OnDestroy {
  @ViewChild('alertsEventsComponent') alertsEventsComponent: AlertseventsComponent;

  breadCrumb = [{ label: 'Alerts & Events', url: Utility.getBaseUIUrl() + 'monitoring/alertsevents' }];

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    // Default tab is alerts
    this.setTabToolbar('alerts');
  }

  setTabToolbar(tabName) {
    let buttons = [];
    if (tabName === 'alerts' && this.uiconfigsService.isAuthorized(UIRolePermissions.monitoringalertpolicy_read)) {
      buttons = [
        {
          cssClass: 'global-button-primary alertsevents-button',
          text: 'ALERT POLICIES',
          callback: () => { this._controllerService.navigate(['/monitoring', 'alertsevents', 'alertpolicies']); }
        },
      ];
    } else if (tabName === 'events') {
      if (this.uiconfigsService.isAuthorized(UIRolePermissions['monitoringarchiverequest_all-actions'] && UIRolePermissions['objstoreobject_create'])) {
        const exportButton = {
          cssClass: 'global-button-primary global-button-padding',
          text: 'EXPORT EVENTS',
          genTooltip: () => this.getTooltip(),
          computeClass: () => this.alertsEventsComponent.eventsTableComponent.shouldEnableButtons && this.alertsEventsComponent.eventsTableComponent.enableExport ? '' : 'global-button-disabled',
          callback: () => {
            this.alertsEventsComponent.eventsTableComponent.anyQueryAfterRefresh = true;
            this.alertsEventsComponent.createArchivePanel();
          }
        };
        buttons.push(exportButton);
      }
      if (this.uiconfigsService.isAuthorized(UIRolePermissions.monitoringeventpolicy_read)) {
        const eventPolicyButton = {
          cssClass: 'global-button-primary alertsevents-button',
          text: 'EVENT POLICIES',
          callback: () => {
            this._controllerService.navigate(['/monitoring', 'alertsevents', 'eventpolicy']);
          }
        };
        buttons.push(eventPolicyButton);
      }
    }
    this._controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: this.breadCrumb
    });
  }

  getTooltip() {
    return this.alertsEventsComponent.eventsTableComponent.enableExport ? 'Ready to submit archive request' : 'Only ONE archive request can be running at a time';
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
