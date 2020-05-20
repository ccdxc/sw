import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation, ViewChild, ChangeDetectionStrategy, ChangeDetectorRef } from '@angular/core';
import { Animations } from '@app/animations';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IMonitoringFwlogPolicy, MonitoringFwlogPolicy, MonitoringFwlogPolicySpec, IMonitoringPSMExportTarget, MonitoringPSMExportTarget } from '@sdk/v1/models/generated/monitoring';
import { SelectItem, MultiSelect } from 'primeng/primeng';
import { SyslogComponent } from '@app/components/shared/syslog/syslog.component';
import { Utility } from '@app/common/Utility';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ValidatorFn, AbstractControl } from '@angular/forms';

@Component({
  selector: 'app-newfwlogpolicy',
  templateUrl: './newfwlogpolicy.component.html',
  styleUrls: ['./newfwlogpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class NewfwlogpolicyComponent extends CreationForm<IMonitoringFwlogPolicy, MonitoringFwlogPolicy> implements OnInit, AfterViewInit {
  public static LOGOPTIONS_ALL = 'FIREWALL_ACTION_ALL';
  public static LOGOPTIONS_NONE = 'FIREWALL_ACTION_NONE';
  public static PSM_TARGET = 'psm-target';

  @ViewChild('syslogComponent') syslogComponent: SyslogComponent;
  @ViewChild('logOptions') logOptionsMultiSelect: MultiSelect;

  @Input() maxTargets: number;
  @Input() existingObjects: MonitoringFwlogPolicy[] = [];

  filterOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringFwlogPolicySpec.propInfo['filter'].enum, [NewfwlogpolicyComponent.LOGOPTIONS_NONE]);

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected _monitoringService: MonitoringService,
    private cdr: ChangeDetectorRef
  ) {
    super(_controllerService, uiconfigsService, MonitoringFwlogPolicy);
  }

  getClassName(): string {
    return this.constructor.name;
  }

  postNgInit() {
    // Existing fwl-policies may not have spec['psm-target'],  so we  build it
    if (!this.newObject.spec[NewfwlogpolicyComponent.PSM_TARGET]) {
      this.newObject.spec[NewfwlogpolicyComponent.PSM_TARGET] =  new MonitoringPSMExportTarget(this.newObject.spec[NewfwlogpolicyComponent.PSM_TARGET]);
      this.newObject.setFormGroupValuesToBeModelValues();
    }
    this.setValidators(this.newObject);
  }
  postViewInit() {
    this.cdr.detectChanges();
  }
  // Empty Hook
  isFormValid(): boolean {
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      this.submitButtonTooltip = 'Policy name is required';
      return false;
    }
    if (this.syslogComponent && this.syslogComponent.syslogRequiredOption && !this.syslogComponent.isSyLogFormValid()['valid']) {
      this.submitButtonTooltip = this.syslogComponent.isSyLogFormValid()['errorMessage'];
      return false;
    }
    if (!this.isInline) {
      if (!this.newObject.$formGroup.get(['meta', 'name']).valid) {
        this.submitButtonTooltip = 'Policy name is invalid or not unique';
        return false;
      }
    }
    return true;
  }
  setValidators(newMonitoringFwlogPolicy: MonitoringFwlogPolicy) {
    newMonitoringFwlogPolicy.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
      this.isNewFlowExportPolicyNameValid(this.existingObjects)]);
  }

  isNewFlowExportPolicyNameValid(existingObjects: IMonitoringFwlogPolicy[]): ValidatorFn {
    return Utility.isModelNameUniqueValidator(existingObjects, 'new-Firewall-Log-Policy-name');
  }
  setToolbar() {
    const currToolbar = this.controllerService.getToolbarData();
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary fwlogpolicies-button fwlogpolicies-button-SAVE',
        text: 'CREATE FIREWALL LOG POLICY',
        callback: () => { this.saveObject(); },
        computeClass: () => this.computeFormSubmitButtonClass(),
        genTooltip: () => this.getSubmitButtonToolTip()
      },
      {
        cssClass: 'global-button-neutral fwlogpolicies-button fwlogpolicies-button-CANCEL',
        text: 'CANCEL',
        callback: () => { this.cancelObject(); }
      },
    ];

    this._controllerService.setToolbarData(currToolbar);
  }

  getObjectValues(): IMonitoringFwlogPolicy {
    const obj = this.newObject.getFormGroupValues();
    const syslogValues = this.syslogComponent.getValues();
    Object.keys(syslogValues).forEach((key) => {
      obj.spec[key] = syslogValues[key];
    });
    return obj;
  }

  createObject(object: IMonitoringFwlogPolicy) {
    return this._monitoringService.AddFwlogPolicy(object);
  }

  updateObject(newObject: IMonitoringFwlogPolicy, oldObject: IMonitoringFwlogPolicy) {
    return this._monitoringService.UpdateFwlogPolicy(oldObject.meta.name, newObject, null, oldObject);
  }

  generateCreateSuccessMsg(object: IMonitoringFwlogPolicy) {
    return 'Created policy ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: IMonitoringFwlogPolicy) {
    return 'Updated policy ' + object.meta.name;
  }

  /**
   * event.itemValue is the changed item. event.value has the selections
   * @param event
   */
  /* VS-124.
     When there is any NONE-ALL options selected, OPTION_ALL will be removed
     When none is seleced, we set OPTION_ALL as default.
  */
  onLogOptionChange(event) {
    const values = this.logOptionsMultiSelect.value;
    if (values.length >= 1 && values.includes(NewfwlogpolicyComponent.LOGOPTIONS_ALL)) {
      // When all is set, we untoggle everything else.
      if (event.itemValue === NewfwlogpolicyComponent.LOGOPTIONS_ALL) {
        this.logOptionsMultiSelect.value = [NewfwlogpolicyComponent.LOGOPTIONS_ALL];
      } else {
        const index = values.indexOf(NewfwlogpolicyComponent.LOGOPTIONS_ALL);
        values.splice(index, 1);
        this.logOptionsMultiSelect.value = values;
      }
    }
  }
}
