import { Component, Input, ViewEncapsulation, ViewChild, ChangeDetectionStrategy, ChangeDetectorRef } from '@angular/core';
import { Animations } from '@app/animations';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import {
  MonitoringFlowExportPolicy, IMonitoringFlowExportPolicy,
  IMonitoringSyslogExport, MonitoringFlowExportPolicySpec,
  MonitoringMatchRule, MonitoringMirrorSessionSpec, MonitoringMirrorSessionSpec_packet_filters
} from '@sdk/v1/models/generated/monitoring';
import { SelectItem } from 'primeng/primeng';
import { SyslogComponent } from '@app/components/shared/syslog/syslog.component';
import { Utility } from '@app/common/Utility';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ValidatorFn, AbstractControl, ValidationErrors, FormArray, FormControl } from '@angular/forms';
import { OrderedItem } from '@app/components/shared/orderedlist/orderedlist.component';
import { IPUtility } from '@app/common/IPUtility';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { SecurityApp } from '@sdk/v1/models/generated/security';
import { SecurityService } from '@app/services/generated/security.service';

@Component({
  selector: 'app-newflowexportpolicy',
  templateUrl: './newflowexportpolicy.component.html',
  styleUrls: ['./newflowexportpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class NewflowexportpolicyComponent extends CreationForm<IMonitoringFlowExportPolicy, MonitoringFlowExportPolicy> {
  @ViewChild('syslogComponent') syslogComponent: SyslogComponent;
  @Input() maxTargets: number;
  @Input() existingObjects: IMonitoringFlowExportPolicy[] = [];
  syslogConfig: IMonitoringSyslogExport;
  formatOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringFlowExportPolicySpec.propInfo['format'].enum);

  ExportPolicyRules: OrderedItem<any>[] = [];

  IPS_LABEL: string = 'IP Addresses';
  IPS_ERRORMSG: string = 'Invalid IP addresses';
  IPS_TOOLTIP: string = 'Type in ip address and hit enter or space key to add more.';
  MACS_LABEL: string = 'MAC Addresses';
  MACS_ERRORMSG: string = 'Invalid MAC addresses. It should be aaaa.bbbb.cccc format.';
  MACS_TOOLTIP: string = 'Type in mac address and hit enter or space key to add more.';
  PORTS_ERRORMSG: string = 'Invalid Protocol/Port';
  PROTS_ERRORMSG: string = 'Invalid Protocol/Port';
  PROTS_TOOLTIP: string = 'Type in valid layer3 or layer 4 protocol and protocol/port, ' +
                          'hit enter or space key to add more. Port can be individual or range.' +
                          'for example: icmp, any/2345, tcp/60001-60100...';

  protoAppOptions: Object[] = [
    { label: 'PROTO-PORTS', value: 'Protocols Ports' },
    { label: 'APPS', value: 'Apps' }];

  securityAppsEventUtility: HttpEventUtility<SecurityApp>;
  securityApps: ReadonlyArray<SecurityApp> = [];
  securityAppOptions: SelectItem[] = [];

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected _monitoringService: MonitoringService,
    protected securityService: SecurityService,
    private cdr: ChangeDetectorRef
  ) {
    super(_controllerService, uiconfigsService, MonitoringFlowExportPolicy);
  }

  getClassName(): string {
    return this.constructor.name;
  }


  getSecurityApps() {
    this.securityAppsEventUtility = new HttpEventUtility<SecurityApp>(SecurityApp, false, null, true); // https://pensando.atlassian.net/browse/VS-93 we want to trim the object
    this.securityApps = this.securityAppsEventUtility.array as ReadonlyArray<SecurityApp>;
    const subscription = this.securityService.WatchApp().subscribe(
      response => {
        this.securityAppsEventUtility.processEvents(response);
        this.securityAppOptions = this.securityApps.map(app => {
          return {
            label: app.meta.name,
            value: app.meta.uuid,
          };
        });
        this.cdr.detectChanges();
      },
      this._controllerService.webSocketErrorHandler('Failed to get Apps')
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  postNgInit() {

    // Field is labeled as exports for this object instead of target
    this.syslogConfig = {
      format: this.newObject.spec.format as any,
      targets: this.newObject.spec.exports,
    };

    this.setValidators(this.newObject);

    if (this.isInline) {
      // process match rules

      this.ExportPolicyRules = [];
      if (this.newObject.spec['match-rules'].length > 0) {
        this.newObject.spec['match-rules'].forEach(rule => {
          this.ExportPolicyRules.push({
            id: Utility.s4(),
            data: { rule },
            inEdit: false
          });
        });
      }

      const targetsArray: FormArray = this.controlAsFormArray(this.newObject.$formGroup.get(['spec', 'exports']));
      if (targetsArray && targetsArray.controls && targetsArray.controls.length > 0) {
        targetsArray.controls.forEach((targetsCtrl: FormControl) => {
          // set gateway validator to null to make the whole form valid
          targetsCtrl.get('gateway').setValidators(null);
        });
      }
    }
    if (this.ExportPolicyRules.length === 0) {
      this.addRule();
    }
  }

  postViewInit() {
    this.cdr.detectChanges();
  }

  addRule() {
    const rule = new MonitoringMatchRule();
    this.ExportPolicyRules.push({
      id: Utility.s4(),
      data: { rule: rule },
      inEdit: false
    });
    this.ExportPolicyRules.forEach((r, i) => {
      if (i === this.ExportPolicyRules.length - 1) {
        r.inEdit = true;
      } else {
        r.inEdit = false;
      }
    });
  }

  deleteRule(index) {
    if (this.ExportPolicyRules.length > 1) {
      this.ExportPolicyRules.splice(index, 1);
    }
  }

  isValidIP(ip: string) {
    return IPUtility.isValidIPWithOptionalMask(ip);
  }

  isValidMAC(mac: string) {
    return Utility.MACADDRESS_REGEX.test(mac);
  }

  isValidProto(chip: string) {
    const arr: string[] = chip.split('/');
    const prot: string = arr[0];
    if (arr.length > 2) {
      return false;
    }
    if (arr.length === 1) {
      return prot === 'any' || prot === 'icmp';
    }
    if (prot !== 'tcp' && prot !== 'udp' && prot !== 'any') {
      return false;
    }
    return Utility.isPortRangeValid(arr[1]);
  }

  areAllRulesEmpty() {
    for (let i = 0; i < this.ExportPolicyRules.length; i++) {
      if (!this.isRuleEmpty(this.ExportPolicyRules[i].data.rule)) {
        return false;
      }
    }
    return true;
  }

  isRuleEmpty(rule: MonitoringMatchRule) {
    const fields = [
      rule.$formGroup.get(['source', 'ip-addresses']),
      rule.$formGroup.get(['source', 'mac-addresses']),
      rule.$formGroup.get(['destination', 'ip-addresses']),
      rule.$formGroup.get(['destination', 'mac-addresses']),
      rule.$formGroup.get(['app-protocol-selectors', 'proto-ports'])
    ];
    for (let i = 0; i < fields.length; i++) {
      if (fields[i].valid && !Utility.isEmpty(fields[i].value)) {
        return false;
      }
    }
    return true;
  }

  setValidators(newMonitoringFlowExportPolicy: MonitoringFlowExportPolicy) {
    newMonitoringFlowExportPolicy.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
      this.isNewFlowExportPolicyNameValid(this.existingObjects)]);

    newMonitoringFlowExportPolicy.$formGroup.get(['spec', 'interval']).setValidators([
      this.isTimeoutValid('interval')
    ]);

    newMonitoringFlowExportPolicy.$formGroup.get(['spec', 'template-interval']).setValidators([
      this.isTimeoutValid('template-interval')
    ]);
  }

  isNewFlowExportPolicyNameValid(existingObjects: IMonitoringFlowExportPolicy[]): ValidatorFn {
    return Utility.isModelNameUniqueValidator(existingObjects, 'newRollout-name');
  }

  /**
   * Validate user inputs
   */
  isFormValid(): boolean {
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      this.submitButtonTooltip = 'Error: Flow export name is required.';
      return false;
    }
    if (!this.isInline) {
      if (!this.newObject.$formGroup.get(['meta', 'name']).valid) {
        this.submitButtonTooltip = 'Flow export name is invalid or not unique';
        return false;
      }
    }
    if (!this.syslogComponent.isSyLogFormValid().valid) {
      this.submitButtonTooltip = this.syslogComponent.isSyLogFormValid().errorMessage;
      return false;
    }

    for (let i = 0; i < this.ExportPolicyRules.length; i++) {
      const rule: MonitoringMatchRule = this.ExportPolicyRules[i].data.rule;
      if (!(rule.$formGroup.get(['source', 'ip-addresses']).valid)) {
        this.submitButtonTooltip =
          'Error: Export ' + (i + 1) + ' source IP addresses are invalid.';
        return false;
      }
      if (!(rule.$formGroup.get(['source', 'mac-addresses']).valid)) {
        this.submitButtonTooltip =
          'Error: Export ' + (i + 1) + ' source MAC addresses are invalid.';
        return false;
      }
      if (!(rule.$formGroup.get(['destination', 'ip-addresses']).valid)) {
        this.submitButtonTooltip =
          'Error: Export ' + (i + 1) + ' destination IP addresses are invalid.';
        return false;
      }
      if (!(rule.$formGroup.get(['destination', 'mac-addresses']).valid)) {
        this.submitButtonTooltip =
          'Error: Export ' + (i + 1) + ' destination MAC addresses are invalid.';
        return false;
      }
      if (!(rule.$formGroup.get(['app-protocol-selectors', 'proto-ports']).valid)) {
        this.submitButtonTooltip =
          'Error: Export ' + (i + 1) + ' protocol/ports are invalid.';
        return false;
      }
    }

    if (!this.newObject.$formGroup.get(['spec', 'interval']).valid) {
      this.submitButtonTooltip = this.newObject.$formGroup.get(['spec', 'interval']).errors.interval.message;
      return false;
    }

    if (!this.newObject.$formGroup.get(['spec', 'template-interval']).valid) {
      this.submitButtonTooltip = this.newObject.$formGroup.get(['spec', 'template-interval']).errors['template-interval'].message;
      return false;
    }
    if (!this.newObject.$formGroup.valid) {
      this.submitButtonTooltip = 'Error: Please correct validation error.';
      return false;
    }
    this.submitButtonTooltip = 'Ready to submit';
    return true;
  }
  setToolbar() {
    const currToolbar = this.controllerService.getToolbarData();
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary flowexportpolicy-button',
        text: 'CREATE FLOW EXPORT POLICY',
        callback: () => { this.saveObject(); },
        computeClass: () => this.computeFormSubmitButtonClass(),
        genTooltip: () => this.getSubmitButtonToolTip()
      },
      {
        cssClass: 'global-button-neutral flowexportpolicy-button',
        text: 'CANCEL',
        callback: () => { this.cancelObject(); }
      },
    ];

    this._controllerService.setToolbarData(currToolbar);
  }

  getObjectValues(): IMonitoringFlowExportPolicy {
    const obj = this.newObject.getFormGroupValues();
    obj.spec['match-rules'] = this.ExportPolicyRules.map(r => {
      return r.data.rule.getFormGroupValues();
    });
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

  getDataArray(data: any, type: string, field: string) {
    return data.rule.$formGroup.get([type, field]).value;

  }

}


