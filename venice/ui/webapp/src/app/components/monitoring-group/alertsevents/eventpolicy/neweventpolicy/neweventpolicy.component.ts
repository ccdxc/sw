import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewChild, ViewEncapsulation, ChangeDetectionStrategy, ChangeDetectorRef } from '@angular/core';
import { Animations } from '@app/animations';
import { FieldselectorComponent } from '@app/components/shared/fieldselector/fieldselector.component';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { MonitoringEventPolicy, IMonitoringEventPolicy, IApiStatus, } from '@sdk/v1/models/generated/monitoring';
import { SyslogComponent } from '@app/components/shared/syslog/syslog.component';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { Utility } from '@app/common/Utility';
import { ValidatorFn } from '@angular/forms';

@Component({
  selector: 'app-neweventpolicy',
  templateUrl: './neweventpolicy.component.html',
  styleUrls: ['./neweventpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class NeweventpolicyComponent extends CreationForm<IMonitoringEventPolicy, MonitoringEventPolicy> implements OnInit, AfterViewInit {
  @ViewChild('fieldSelector') fieldSelector: FieldselectorComponent;
  @ViewChild('syslogComponent') syslogComponent: SyslogComponent;
  @Input() existingObjects: MonitoringEventPolicy[] = [];
  @Input() isInline: boolean = false;
  @Input() maxTargets: number;

  constructor(
    protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected _monitoringService: MonitoringService,
    private cdr: ChangeDetectorRef
  ) {
    super(_controllerService, uiconfigsService, MonitoringEventPolicy);
  }

  getClassName(): string {
    return this.constructor.name;
  }

  // Empty Hook
  postNgInit() {
    this.setValidators(this.newObject);
  }

  // Empty Hook
  isFormValid(): boolean {
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      this.submitButtonTooltip = 'Eventpolicy name is required';
      return false;
    }

    if (!this.syslogComponent.isSyLogFormValid()['valid']) {
      this.submitButtonTooltip = this.syslogComponent.isSyLogFormValid()['errorMessage'];
      return false;
    }
    if (!this.isInline) {
      if (!this.newObject.$formGroup.get(['meta', 'name']).valid) {
        this.submitButtonTooltip = 'Eventpolicy name is invalid or not unique';
        return false;
      }
    }
    this.submitButtonTooltip = 'Ready to submit';
    return true;
  }
  postViewInit() {
    this.cdr.detectChanges();
  }

  setValidators(newMonitoringFlowExportPolicy: MonitoringEventPolicy) {
    newMonitoringFlowExportPolicy.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
      this.isNewFlowExportPolicyNameValid(this.existingObjects)]);
  }

  isNewFlowExportPolicyNameValid(existingObjects: IMonitoringEventPolicy[]): ValidatorFn {
    return Utility.isModelNameUniqueValidator(existingObjects, 'newEvent-Policy-name');
  }
  setToolbar() {
    const currToolbar = this.controllerService.getToolbarData();
    this.oldButtons = currToolbar.buttons;
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary eventpolicy-button',
        text: 'CREATE EVENT POLICY',
        callback: () => { this.saveObject(); },
        computeClass: () => this.computeFormSubmitButtonClass(),
        genTooltip: () => this.getSubmitButtonToolTip()
      },
      {
        cssClass: 'global-button-neutral eventpolicy-button',
        text: 'CANCEL',
        callback: () => { this.cancelObject(); }
      },
    ];

    this._controllerService.setToolbarData(currToolbar);
  }

  getObjectValues(): IMonitoringEventPolicy {
    const obj = this.newObject.getFormGroupValues();
    const syslogValues = this.syslogComponent.getValues();
    Object.keys(syslogValues).forEach((key) => {
      obj.spec[key] = syslogValues[key];
    });
    return obj;
  }

  createObject(object: IMonitoringEventPolicy) {
    return this._monitoringService.AddEventPolicy(object);
  }

  updateObject(newObject: IMonitoringEventPolicy, oldObject: IMonitoringEventPolicy) {
    return this._monitoringService.UpdateEventPolicy(oldObject.meta.name, newObject, null, oldObject);
  }

  generateCreateSuccessMsg(object: IMonitoringEventPolicy) {
    return 'Created policy ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: IMonitoringEventPolicy) {
    return 'Updated policy ' + object.meta.name;
  }

}
