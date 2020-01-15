import { AfterViewInit, Component, Input, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { FieldselectorComponent } from '@app/components/shared/fieldselector/fieldselector.component';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IMonitoringAlertDestination, IMonitoringAlertPolicy, MonitoringAlertPolicy, MonitoringAlertPolicySpec } from '@sdk/v1/models/generated/monitoring';
import { SelectItem } from 'primeng/primeng';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';

@Component({
  selector: 'app-neweventalertpolicy',
  templateUrl: './neweventalertpolicy.component.html',
  styleUrls: ['./neweventalertpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class NeweventalertpolicyComponent extends CreationForm<IMonitoringAlertPolicy, MonitoringAlertPolicy> implements OnInit, AfterViewInit {
  @ViewChild('fieldSelector') fieldSelector: FieldselectorComponent;

  @Input() destinations: IMonitoringAlertDestination[] = [];

  alertOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringAlertPolicySpec.propInfo['severity'].enum);

  destinationOptions: SelectItem[] = [];

  oldButtons: ToolbarButton[] = [];

  constructor(protected controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected _monitoringService: MonitoringService,
  ) {
    super(controllerService, uiconfigsService, MonitoringAlertPolicy);
  }

  postNgInit() {
    if (this.objectData == null) {
      this.newObject.spec.enable = true;
    }

    this.newObject.$formGroup.get(['spec', 'resource']).setValue('Event');

    this.destinations.forEach((destination) => {
      this.destinationOptions.push({
        label: destination.meta.name,
        value: destination.meta.name,
      });
    });
    if (this.newObject.$formGroup.get(['spec', 'destinations']).value.length === 0) {
     this.newObject.$formGroup.get(['spec', 'destinations']).disable();
    }
  }

  getClassName(): string {
    return this.constructor.name;
  }

  // Empty Hook
  isFormValid(): boolean {
    return true;
  }

  setToolbar() {
    const currToolbar = this.controllerService.getToolbarData();
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary eventalertpolicies-button eventalertpolicies-button-eventalertpolicy-SAVE',
        text: 'CREATE ALERT POLICY',
        callback: () => { this.saveObject(); },
        computeClass: () => this.computeButtonClass()
      },
      {
        cssClass: 'global-button-neutral eventalertpolicies-button eventalertpolicies-button-eventalertpolicy-CANCEL',
        text: 'CANCEL',
        callback: () => { this.cancelObject(); }
      },
    ];

    this.controllerService.setToolbarData(currToolbar);
  }

  getObjectValues(): IMonitoringAlertPolicy {
    const obj = this.newObject.getFormGroupValues();
    obj.spec.requirements = this.fieldSelector.getValues();
    return obj;
  }

  createObject(object: IMonitoringAlertPolicy) {
    return this._monitoringService.AddAlertPolicy(object);
  }

  updateObject(newObject: IMonitoringAlertPolicy, oldObject: IMonitoringAlertPolicy) {
    return this._monitoringService.UpdateAlertPolicy(oldObject.meta.name, newObject, null, oldObject);
  }

  generateCreateSuccessMsg(object: IMonitoringAlertPolicy) {
    return 'Created policy ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: IMonitoringAlertPolicy) {
    return 'Updated policy ' + object.meta.name;
  }

}
