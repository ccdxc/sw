import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation, ViewChild } from '@angular/core';
import { Animations } from '@app/animations';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { MonitoringFlowExportPolicy, IMonitoringFlowExportPolicy, IMonitoringSyslogExport, MonitoringFlowExportPolicySpec } from '@sdk/v1/models/generated/monitoring';
import { SelectItem } from 'primeng/primeng';
import { SyslogComponent } from '@app/components/monitoring-group/syslog/syslog.component';
import { MatchruleComponent } from '@app/components/monitoring-group/matchrule/matchrule.component';
import { Utility } from '@app/common/Utility';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ValidatorFn } from '@angular/forms';

@Component({
  selector: 'app-newflowexportpolicy',
  templateUrl: './newflowexportpolicy.component.html',
  styleUrls: ['./newflowexportpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class NewflowexportpolicyComponent extends CreationForm<IMonitoringFlowExportPolicy, MonitoringFlowExportPolicy> implements OnInit, AfterViewInit {
  @ViewChild('matchRules') matchRulesComponent: MatchruleComponent;
  @ViewChild('syslogComponent') syslogComponent: SyslogComponent;

  @Input() maxTargets: number;
  @Input() existingObjects: IMonitoringFlowExportPolicy[] = [];

  syslogConfig: IMonitoringSyslogExport;

  formatOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringFlowExportPolicySpec.propInfo['format'].enum);
  validationMessage: string = null;

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected _monitoringService: MonitoringService,
  ) {
    super(_controllerService, uiconfigsService, MonitoringFlowExportPolicy);
  }

  getClassName(): string {
    return this.constructor.name;
  }

  postNgInit() {
    // Field is labeled as exports for this object instead of target
    this.syslogConfig = {
      format: this.newObject.spec.format as any,
      targets: this.newObject.spec.exports,
    };
    if (!this.isInline) {
      this.setValidators(this.newObject);
    }
  }

  setValidators(newMonitoringFlowExportPolicy: MonitoringFlowExportPolicy) {
    newMonitoringFlowExportPolicy.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
      this.isNewFlowExportPolicyNameValid(this.existingObjects)]);
  }

  isNewFlowExportPolicyNameValid(existingObjects: IMonitoringFlowExportPolicy[]): ValidatorFn {
    // checks if name field is valid
    return Utility.isModelNameUniqueValidator(existingObjects, 'newRollout-name');
  }

  /**
   * Validate uesr inputs
   */
  isFormValid(): boolean {
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      this.validationMessage = 'Flow export name is required.';
      return false;
    }
    if (!this.areMatchRulesValid()) {
      this.validationMessage = 'Either configure at least one match rule or no match rules.';
      return false;
    }
    if (!this.areExportTargetsValid()) {
      this.validationMessage = 'At least one target config is needed.';
      return false;
    }
    if (!this.newObject.$formGroup.valid) {
      this.validationMessage = 'Please correct validation error.';
      return false;
    }
    return true;
  }

  /**
   * This API validate whether the match-rule input is valid
   * VS-858
   * VS-893
   * User can specify no rule, but can not have some empty rules.
   */
  areMatchRulesValid(): boolean {
    return this.matchRulesComponent.isValid();
  }

  areExportTargetsValid(): boolean {
   return this.syslogComponent.atLeastOneTargetFilled();
  }

  setToolbar() {
    const currToolbar = this.controllerService.getToolbarData();
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary flowexportpolicy-button',
        text: 'CREATE FLOW EXPORT POLICY',
        callback: () => { this.saveObject(); },
        computeClass: () => this.computeButtonClass(),
        genTooltip: () => this.getTooltip(),
      },
      {
        cssClass: 'global-button-neutral flowexportpolicy-button',
        text: 'CANCEL',
        callback: () => { this.cancelObject(); }
      },
    ];

    this._controllerService.setToolbarData(currToolbar);
  }

  getTooltip(): string {
    this.isFormValid();
    return Utility.isEmpty(this.validationMessage) ? 'Ready to save flow export policy' : this.validationMessage;
  }

  getObjectValues(): IMonitoringFlowExportPolicy {
    const obj = this.newObject.getFormGroupValues();
    obj.spec['match-rules'] = this.matchRulesComponent.getValues();

    const syslogValues = this.syslogComponent.getValues();
    Object.keys(syslogValues).forEach((key) => {
      if (key === 'targets') {
        obj.spec['exports'] = syslogValues[key];
      } else {
        obj.spec[key] = syslogValues[key];
      }
    });
    return obj;
  }

  createObject(object: IMonitoringFlowExportPolicy) {
    return this._monitoringService.AddFlowExportPolicy(object);
  }

  updateObject(newObject: IMonitoringFlowExportPolicy, oldObject: IMonitoringFlowExportPolicy) {
    return this._monitoringService.UpdateFlowExportPolicy(oldObject.meta.name, newObject, null, oldObject);
  }

  generateCreateSuccessMsg(object: IMonitoringFlowExportPolicy) {
    return 'Created policy ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: IMonitoringFlowExportPolicy) {
    return 'Updated policy ' + object.meta.name;
  }

}
