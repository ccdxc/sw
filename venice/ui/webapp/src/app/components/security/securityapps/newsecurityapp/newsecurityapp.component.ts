import { Component, OnInit, Input, Output, DoCheck, EventEmitter, AfterViewInit, ViewEncapsulation,  } from '@angular/core';
import { Animations } from '@app/animations';
import { IApiStatus, ISecurityApp, SecurityApp, SecurityAppSpec, SecurityALG, SecurityALG_type, SecuritySunrpc, SecurityMsrpc} from '@sdk/v1/models/generated/security';
import { Observable } from 'rxjs';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { SelectItem, MultiSelect } from 'primeng/primeng';
import { Utility } from '@app/common/Utility';
import { FormArray, FormGroup, AbstractControl, ValidatorFn, ValidationErrors } from '@angular/forms';
import { SecurityAppOptions} from '@app/components/security';
import { SecurityProtoPort } from '@sdk/v1/models/generated/search';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';

@Component({
  selector: 'app-newsecurityapp',
  templateUrl: './newsecurityapp.component.html',
  styleUrls: ['./newsecurityapp.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewsecurityappComponent extends CreationForm<ISecurityApp, SecurityApp> implements OnInit, AfterViewInit {

  newApp: SecurityApp;
  @Input() existingApps: SecurityApp[] = [];
  securityForm: FormGroup;
  secOptions: SelectItem[] = Utility.convertEnumToSelectItem(SecurityAppOptions);
  pickedOption: String;
  protoandports: any = null;
  securityOptions = SecurityAppOptions;
  algEnumOptions = SecurityALG_type;
  algOptions: SelectItem[] = Utility.convertEnumToSelectItem(SecurityALG.propInfo['type'].enum, [SecurityALG_type.rtsp, SecurityALG_type.tftp]);
  selectedType = SecurityALG_type.icmp;
  sunRPCTargets: any = null;
  msRPCTargets: any = null;


  constructor(protected _controllerService: ControllerService,
    protected _securityService: SecurityService,
    protected uiconfigsService: UIConfigsService
  ) {
    super(_controllerService, uiconfigsService, SecurityApp);
  }

  postNgInit() {
    if (this.objectData != null) {
      this.setRadio();
    } else {
      this.pickedOption = SecurityAppOptions.PROTOCOLSANDPORTS;
    }
    this.securityForm = this.newObject.$formGroup;
    this.setUpTargets();
    const dnsTimeout: AbstractControl = this.securityForm.get(['spec', 'alg', 'dns', 'query-response-timeout']);
    this.addTimeoutValidator(dnsTimeout, 'dnsTimeout');
  }

  setCustomValidation() {
    this.newObject.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
      this.isAppNameValid(this.existingApps)]);
  }

  generateCreateSuccessMsg(object: ISecurityApp) {
    return 'Created app ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: ISecurityApp) {
    return 'Updated app ' + object.meta.name;
  }

  getClassName(): string {
    return this.constructor.name;
  }

  getObjectValues(): ISecurityApp {
    const obj = this.newObject.getFormGroupValues();
    return obj;
  }
  // tcp,icmp,udp, any + 1< x < 255
  updateObject(newObject: ISecurityApp, oldObject: ISecurityApp) {
    return this._securityService.UpdateApp(oldObject.meta.name, newObject, null, oldObject);
  }

  setToolbar() {
    if (!this.isInline) {
      // If it is not inline, we change the toolbar buttons, and save the old one
      // so that we can set it back when we are done
      const currToolbar = this._controllerService.getToolbarData();
      currToolbar.buttons = [
        {
          cssClass: 'global-button-primary newsecurityapp-button',
          text: 'CREATE APP ',
          callback: () => { this.savePolicy(); },
          computeClass: () => this.computeButtonClass()
        },
        {
          cssClass: 'global-button-primary newsecurityapp-button',
          text: 'CANCEL',
          callback: () => { this.cancelObject(); }
        },
      ];

      this._controllerService.setToolbarData(currToolbar);
    }
  }

  isAppNameValid(existingApps: SecurityApp[]): ValidatorFn {
    return Utility.isModelNameUniqueValidator(existingApps, 'security-app-name');
  }

  setUpTargets() {
    const tempProto: any = this.securityForm.get(['spec', 'proto-ports']);
    if (!tempProto.controls || tempProto.controls.length === 0) {
      this.addProtoTarget();
    }
    this.protoandports = (<any>this.securityForm.get(['spec', 'proto-ports'])).controls;
    const tempSun: any = this.securityForm.get(['spec', 'alg', 'sunrpc']);
    if (!tempSun.controls || tempSun.controls.length === 0) {
      this.addSunRPCTarget();
    }
    this.sunRPCTargets = (<any>this.securityForm.get(['spec', 'alg', 'sunrpc'])).controls;
    const tempMSRPC: any = this.securityForm.get(['spec', 'alg', 'msrpc']);
    if (!tempMSRPC.controls || tempMSRPC.controls.length === 0) {
      this.addMSRPCTarget();
    }
    this.msRPCTargets = (<any>this.securityForm.get(['spec', 'alg', 'msrpc'])).controls;

  }

  createObject(object: ISecurityApp) {
    return this._securityService.AddApp(object);
  }

  setRadio() {
    if (this.objectData.spec['alg'] == null) {
      this.pickedOption = SecurityAppOptions.PROTOCOLSANDPORTS;
    } else if (this.objectData.spec['alg'] != null && this.objectData.spec['proto-ports'] == null) {
      this.pickedOption = SecurityAppOptions.ALGONLY;
      this.selectedType = this.objectData.spec.alg.type;
    } else if (this.objectData.spec['alg'] != null && this.objectData.spec['proto-ports'] != null) {
      this.pickedOption = SecurityAppOptions.BOTH;
      this.selectedType = this.objectData.spec.alg.type;
    } else {
      this.pickedOption = SecurityAppOptions.PROTOCOLSANDPORTS;
    }
  }

  addSunRPCTarget() {
    const tempTargets = this.securityForm.get(['spec', 'alg', 'sunrpc']) as FormArray;
    const newFormGroup: FormGroup = new SecuritySunrpc().$formGroup;
    const ctrl: AbstractControl = newFormGroup.get(['timeout']);
    this.addTimeoutValidator(ctrl, 'sunrpcTimeout');
    tempTargets.insert(0, newFormGroup);
  }

  addProtoTarget() {
    const tempTargets = this.securityForm.get(['spec', 'proto-ports']) as FormArray;
    const newFormGroup: FormGroup = new SecurityProtoPort().$formGroup;
    const ctrl: AbstractControl = newFormGroup.get(['ports']);
    ctrl.disable();
    this.addPortsValidator(ctrl);
    tempTargets.insert(0, newFormGroup);
  }

  addMSRPCTarget() {
    const tempTargets = this.securityForm.get(['spec', 'alg', 'msrpc']) as FormArray;
    const newFormGroup: FormGroup = new SecurityMsrpc().$formGroup;
    const ctrl: AbstractControl = newFormGroup.get(['timeout']);
    this.addTimeoutValidator(ctrl, 'msrpcTimeout');
    tempTargets.insert(0, newFormGroup);
  }

  removeProtoTarget(index) {
    const tempTargets = this.securityForm.get(['spec', 'proto-ports']) as FormArray;
    if (tempTargets.length > 1) {
      tempTargets.removeAt(index);
    }
  }

  removeSunRPCTarget(index) {
    const tempTargets = this.securityForm.get(['spec', 'alg', 'sunrpc']) as FormArray;
    if (tempTargets.length > 1) {
      tempTargets.removeAt(index);
    }
  }

  removeMSRPCTarget(index) {
    const tempTargets = this.securityForm.get(['spec', 'alg', 'msrpc']) as FormArray;
    if (tempTargets.length > 1) {
      tempTargets.removeAt(index);
    }
  }

  processALGValues() {
    if (this.pickedOption === SecurityAppOptions.PROTOCOLSANDPORTS) {
      this.resetDNSValues();
      this.resetFTPValues();
      this.resetSunRPC();
      this.resetMSRPC();
      this.resetICMPValues();
      this.securityForm.get(['spec', 'alg', 'type']).setValue(SecurityALG_type.icmp);
      return;
    }
    if (this.selectedType === SecurityALG_type.icmp) {
      this.resetDNSValues();
      this.resetFTPValues();
      this.resetSunRPC();
      this.resetMSRPC();
    } else if (this.selectedType === SecurityALG_type.dns) {
      this.resetICMPValues();
      this.resetFTPValues();
      this.resetSunRPC();
      this.resetMSRPC();
    } else if (this.selectedType === SecurityALG_type.ftp) {
      this.resetICMPValues();
      this.resetDNSValues();
      this.resetSunRPC();
      this.resetMSRPC();
    } else if (this.selectedType === SecurityALG_type.msrpc) {
      this.resetICMPValues();
      this.resetDNSValues();
      this.resetFTPValues();
      this.resetSunRPC();
    } else if (this.selectedType === SecurityALG_type.sunrpc) {
      this.resetICMPValues();
      this.resetDNSValues();
      this.resetFTPValues();
      this.resetMSRPC();
    }
  }

  processFormValues() {
    if (this.pickedOption === SecurityAppOptions.ALGONLY) {
      this.resetProto();
    }
    this.processALGValues();

  }

  resetICMPValues() {
    this.securityForm.get(['spec', 'alg', 'icmp', 'type']).setValue(null);
    this.securityForm.get(['spec', 'alg', 'icmp', 'code']).setValue(null);
  }

  resetDNSValues() {
    this.securityForm.get(['spec', 'alg', 'dns', 'drop-multi-question-packets']).setValue(null);
    this.securityForm.get(['spec', 'alg', 'dns', 'drop-large-domain-name-packets']).setValue(null);
    this.securityForm.get(['spec', 'alg', 'dns', 'drop-long-label-packets']).setValue(null);
    this.securityForm.get(['spec', 'alg', 'dns', 'max-message-length']).setValue(null);
    this.securityForm.get(['spec', 'alg', 'dns', 'query-response-timeout']).setValue(null);
  }

  resetFTPValues() {
    this.securityForm.get(['spec', 'alg', 'ftp', 'allow-mismatch-ip-address']).setValue(null);
  }

  onTypeChange($event) {
    this.selectedType = $event.value;
  }

  resetProto() {
    const tempproto = this.securityForm.get(['spec', 'proto-ports']) as FormArray;
    while (tempproto && tempproto.length !== 0) {
      tempproto.removeAt(0);
    }
  }

  resetSunRPC() {
    const tempSunrpc = this.securityForm.get(['spec', 'alg', 'sunrpc']) as FormArray;
    while (tempSunrpc && tempSunrpc.length !== 0) {
      tempSunrpc.removeAt(0);
    }
  }

  resetMSRPC() {
    const tempMSRPC = this.securityForm.get(['spec', 'alg', 'msrpc']) as FormArray;
    while (tempMSRPC && tempMSRPC.length !== 0) {
      tempMSRPC.removeAt(0);
    }
  }

  savePolicy() {
    this.processFormValues();
    this.saveObject();
  }

  isFormValid(): boolean {
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      return false;
    }
    if (!this.isInline) {
      if (this.newObject.$formGroup.get('meta.name').status !== 'VALID') {
        return false;
      }
    }
    if (this.pickedOption === SecurityAppOptions.PROTOCOLSANDPORTS) {
      return this.validatingProtoInputs();
    } else if (this.pickedOption === SecurityAppOptions.ALGONLY) {
      return this.validatingALGinputs();
    } else {
      return this.validatingProtoInputs() && this.validatingALGinputs();
    }
  }

  validatingProtoInputs() {
    const tempProto: FormArray = this.securityForm.get(['spec', 'proto-ports']) as FormArray;
    const formGroups: FormGroup[] = tempProto.controls as FormGroup[];
    for (let i = 0; i < formGroups.length; i++) {
      const formGroup: FormGroup = formGroups[i];
      if (!formGroup.value || Utility.isEmpty(formGroup.value.protocol, true)) {
        return false;
      }
      if ((formGroup.value.protocol === 'tcp' || formGroup.value.protocol === 'udp')
          && Utility.isEmpty(formGroup.value.ports, true)) {
        return false;
      }
      if (!formGroup.valid) { // validate error
        return false;
      }
    }
    return true;
  }

  validatingALGinputs() {
    if (this.selectedType === SecurityALG_type.icmp) {
      if (this.securityForm.get(['spec', 'alg', 'icmp']).value == null || this.securityForm.get(['spec', 'alg', 'icmp']).value.type == null
      || this.securityForm.get(['spec', 'alg', 'icmp']).value.code == null) {
        return false;
      } else if (this.securityForm.get(['spec', 'alg', 'icmp']).value.code === '' || this.securityForm.get(['spec', 'alg', 'icmp']).value.type === '') {
        return false;
      }
      return true;
    } else {
      return true;
    }
  }

  onProtocolChange(event: any, formGroup: any): void {
    let val = formGroup.get(['protocol']).value;
    const portsField: AbstractControl = formGroup.get(['ports']);
    if (val) {
      val = val.trim();
    }
    if (val !== 'tcp' && val !== 'udp') {
      portsField.setValue(null);
      portsField.disable();
    } else {
      portsField.enable();
    }
  }

  isPortRequired(formGroup: any): boolean {
    const val = formGroup.get(['ports']).value;
    if (val && val.trim()) {
      return false;
    }
    const protocol = formGroup.get(['protocol']).value;
    return protocol && (protocol.trim() === 'tcp' || protocol.trim() === 'udp');
  }

  isPortsFieldValid(): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      const val: string = control.value;
      if (!val || !val.trim()) {
        return null;
      }
      const errorMsg = Utility.isPortsValid(val);
      if (errorMsg) {
        return {
          fieldPort: {
            required: false,
            message: errorMsg
          }
        };
      }
      return null;
    };
  }

  addPortsValidator(ctrl: AbstractControl): void {
    if (!ctrl.validator) {
      ctrl.setValidators([this.isPortsFieldValid()]);
    } else {
      ctrl.setValidators([ctrl.validator, this.isPortsFieldValid()]);
    }
  }

  isTimeoutValid(fieldName): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      const val: string = control.value;
      if (!val) {
        return null;
      }
      if (!Utility.isTimeoutValid(val)) {
        return {
          fieldname: {
            required: false,
            message: 'Invalid time out value. Only h, m, s, ms, us, and ns are allowed'
          }
        };
      }
      return null;
    };
  }

  addTimeoutValidator(ctrl: AbstractControl, fieldName: string): void {
    if (!ctrl.validator) {
      ctrl.setValidators([this.isTimeoutValid(fieldName)]);
    } else {
      ctrl.setValidators([ctrl.validator, this.isTimeoutValid(fieldName)]);
    }
  }
}

