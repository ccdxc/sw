import { Component, OnInit, Input, Output, DoCheck, EventEmitter, AfterViewInit, ViewEncapsulation,  } from '@angular/core';
import { Animations } from '@app/animations';
import { IApiStatus, ISecurityApp, SecurityApp, SecurityAppSpec, SecurityALG, SecurityALG_type, SecuritySunrpc, SecurityMsrpc} from '@sdk/v1/models/generated/security';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { SelectItem, MultiSelect } from 'primeng/primeng';
import { Utility } from '@app/common/Utility';
import { FormArray, FormGroup, AbstractControl, ValidatorFn, ValidationErrors, FormControl } from '@angular/forms';
import { SecurityAppOptions} from '@app/components/security';
import { SecurityProtoPort } from '@sdk/v1/models/generated/search';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { maxValueValidator, minValueValidator } from '@sdk/v1/utils/validators';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';

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
  algOptions: SelectItem[] = Utility.convertEnumToSelectItem(SecurityALG.propInfo['type'].enum);

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
    this.addFieldValidator(dnsTimeout, this.isTimeoutValid('dnsTimeout'));
    const icmpType: AbstractControl = this.securityForm.get(['spec', 'alg', 'icmp', 'type']);
    this.addFieldValidator(icmpType, minValueValidator( 0));
    this.addFieldValidator(icmpType, maxValueValidator(255));
    const icmpCode: AbstractControl = this.securityForm.get(['spec', 'alg', 'icmp', 'code']);
    this.addFieldValidator(icmpCode, minValueValidator(0));
    this.addFieldValidator(icmpCode, maxValueValidator(18));
    const dnsMaxMsgLen: AbstractControl = this.securityForm.get(['spec', 'alg', 'dns', 'max-message-length']);
    this.addFieldValidator(dnsMaxMsgLen, minValueValidator(1));
    this.addFieldValidator(dnsMaxMsgLen, maxValueValidator(8129));
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
    if (obj.spec.alg && obj.spec.alg.icmp) {
      if (obj.spec.alg.icmp.type || obj.spec.alg.icmp.type === 0) {
        obj.spec.alg.icmp.type = obj.spec.alg.icmp.type.toString();
      }
      if (obj.spec.alg.icmp.code || obj.spec.alg.icmp.code === 0) {
        obj.spec.alg.icmp.code = obj.spec.alg.icmp.code.toString();
      }
    }
    return obj;
  }
  // tcp,icmp,udp, any + 1< x < 255
  updateObject(newObject: ISecurityApp, oldObject: ISecurityApp) {
    return this._securityService.UpdateApp(oldObject.meta.name, newObject, null, oldObject, true, false);
  }

  setToolbar() {
    if (!this.isInline && this.uiconfigsService.isAuthorized(UIRolePermissions.securityapp_create)) {
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
    return this._securityService.AddApp(object, '', true, false);
  }

  setRadio() {
    if (!this.objectData.spec['alg']) {
      this.pickedOption = SecurityAppOptions.PROTOCOLSANDPORTS;
    } else if (this.objectData.spec['alg'] && !this.objectData.spec['proto-ports']) {
      this.pickedOption = SecurityAppOptions.ALGONLY;
      this.selectedType = this.objectData.spec.alg.type;
    } else if (this.objectData.spec['alg'] && this.objectData.spec['proto-ports']) {
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
    this.addFieldValidator(ctrl, this.isTimeoutValid('sunrpcTimeout'));
    tempTargets.insert(0, newFormGroup);
  }

  addProtoTarget() {
    const tempTargets = this.securityForm.get(['spec', 'proto-ports']) as FormArray;
    const newFormGroup: FormGroup = new SecurityProtoPort().$formGroup;
    const ctrl: AbstractControl = newFormGroup.get(['ports']);
    ctrl.disable();
    this.addFieldValidator(ctrl, this.isPortsFieldValid());
    const ctrl2: AbstractControl = newFormGroup.get(['protocol']);
    this.addFieldValidator(ctrl2, this.isProtocolFieldValid());
    tempTargets.insert(0, newFormGroup);
  }

  addMSRPCTarget() {
    const tempTargets = this.securityForm.get(['spec', 'alg', 'msrpc']) as FormArray;
    const newFormGroup: FormGroup = new SecurityMsrpc().$formGroup;
    const ctrl: AbstractControl = newFormGroup.get(['timeout']);
    this.addFieldValidator(ctrl, this.isTimeoutValid('msrpcTimeout'));
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
      this.resetALG();
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
    } else if (this.selectedType === SecurityALG_type.rtsp ||
              this.selectedType === SecurityALG_type.tftp) {
      this.resetICMPValues();
      this.resetDNSValues();
      this.resetFTPValues();
      this.resetSunRPC();
      this.resetMSRPC();
    }
  }

  processFormValues() {
    if (this.pickedOption === SecurityAppOptions.ALGONLY) {
      this.resetProto();
    }
    this.processALGValues();

  }

  resetALG() {
    this.securityForm.get(['spec', 'alg', 'type']).setValue(null);
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

  /*
   * when user choose alg, ui will automatically create proto-ports based on
   * which alg it is. Basically Naple card api demands both alg and proto-ports.
   * Backend does not create default proto-ports by the alg type, they ask ui and
   * rest api to do that.
   * For icmp, user does not need to specify proto-ports. Backend will create
   * protocol: icmp port null for this. That is the only proto-portsbackend creates
   * For ftp: UI creates     Protocol: tcp Port 21
   * For dns: UI creates     Protocol: udp Port 53
   * For tftp: UI creates    Protocol: udp Port 69
   * For rstp: UI creates    Protocol: tcp Port 554
   * For sunrpc and msrpc    user has to specify because protocol can be either ftp or udp
   *                         plus ports should be a range
   */
  onTypeChange($event) {
    this.selectedType = $event.value;
    if (this.selectedType !== SecurityALG_type.icmp) {
      this.pickedOption = SecurityAppOptions.BOTH;
      this.resetProto();
      this.addProtoTarget();
      const firstProtGroup: FormGroup = this.securityForm.get(['spec', 'proto-ports', 0]) as FormGroup;
      if (this.selectedType === SecurityALG_type.ftp) {
        firstProtGroup.get(['protocol']).setValue('tcp');
        firstProtGroup.get(['protocol']).updateValueAndValidity();
        firstProtGroup.get(['ports']).enable();
        firstProtGroup.get(['ports']).setValue('21');
      } else if (this.selectedType === SecurityALG_type.dns) {
        firstProtGroup.get(['protocol']).setValue('udp');
        firstProtGroup.get(['protocol']).updateValueAndValidity();
        firstProtGroup.get(['ports']).enable();
        firstProtGroup.get(['ports']).setValue('53');
      } else if (this.selectedType === SecurityALG_type.tftp) {
        firstProtGroup.get(['protocol']).setValue('udp');
        firstProtGroup.get(['protocol']).updateValueAndValidity();
        firstProtGroup.get(['ports']).enable();
        firstProtGroup.get(['ports']).setValue('69');
      } else if (this.selectedType === SecurityALG_type.rtsp) {
        firstProtGroup.get(['protocol']).setValue('tcp');
        firstProtGroup.get(['protocol']).updateValueAndValidity();
        firstProtGroup.get(['ports']).enable();
        firstProtGroup.get(['ports']).setValue('554');
      }
    } else {
      this.pickedOption = SecurityAppOptions.ALGONLY;
    }
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
      return this.validatingProtoInputs() && !this.isProtoInputsEmpty();
    } else if (this.pickedOption === SecurityAppOptions.ALGONLY) {
      return this.validatingALGinputs();
    } else {
      if (!this.validatingALGinputs()) {
        return false;
      }
      if (!this.validatingProtoInputs()) {
        return false;
      }
      if (this.selectedType === SecurityALG_type.icmp) {
        return true;
      }
      return !this.isProtoInputsEmpty();
    }
  }

  isProtoInputsEmpty() {
    const tempProto: FormArray = this.controlAsFormArray(
      this.securityForm.get(['spec', 'proto-ports']));
    for (let i = 0; i < tempProto.controls.length; i++) {
      const formGroup: FormGroup = tempProto.controls[i] as FormGroup;
      if (formGroup.value && formGroup.value.protocol) {
        if (formGroup.value.protocol !== 'tcp' &&
            formGroup.value.protocol !== 'udp') {
          return false;
        }
        if (formGroup.valid) {
          return false;
        }
      }
    }
    return true;
  }

  validatingProtoInputs() {
    const tempProto: FormArray = this.securityForm.get(['spec', 'proto-ports']) as FormArray;
    const formGroups: FormGroup[] = tempProto.controls as FormGroup[];
    for (let i = 0; i < formGroups.length; i++) {
      const formGroup: FormGroup = formGroups[i];
      if (formGroup.value && (formGroup.value.protocol === 'tcp' || formGroup.value.protocol === 'udp')
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
      if (!this.securityForm.get(['spec', 'alg', 'icmp']).valid) {
        return false;
      }
    } else if (this.selectedType === SecurityALG_type.dns) {
      if (!this.securityForm.get(['spec', 'alg', 'dns']).valid) {
        return false;
      }
    } else if (this.selectedType === SecurityALG_type.sunrpc) {
      const formArray: FormArray = this.controlAsFormArray(
        this.securityForm.get(['spec', 'alg', 'sunrpc']));
      for (let i = 0; i < formArray.controls.length; i++) {
        const formGroup: FormGroup = formArray.controls[i] as FormGroup;
        if (!formGroup.value || !formGroup.value['program-id'] ||
            !formGroup.value.timeout) {
          return false;
        }
        if (!formGroup.valid) {
          return false;
        }
      }
    } else if (this.selectedType === SecurityALG_type.msrpc) {
      const formArray: FormArray = this.controlAsFormArray(
        this.securityForm.get(['spec', 'alg', 'msrpc']));
      for (let i = 0; i < formArray.controls.length; i++) {
        const formGroup: FormGroup = formArray.controls[i] as FormGroup;
        if (!formGroup.value || !formGroup.value['program-uuid'] ||
            !formGroup.value.timeout) {
          return false;
        }
        if (!formGroup.valid) {
          return false;
        }
      }
    }
    return true;
  }

  isPortRequired(formGroup: any): boolean {
    const protocol = formGroup.get(['protocol']).value;
    const portsCtrl: FormControl = formGroup.get(['ports']);
    const shouldEnable: boolean = protocol && (protocol.trim() === 'tcp' || protocol.trim() === 'udp' || protocol.trim() === 'any');
    if (shouldEnable) {
      portsCtrl.enable();
    } else {
      portsCtrl.setValue(null);
      portsCtrl.disable();
    }
    const val = portsCtrl.value;
    if (val && val.trim()) {
      return false;
    }
    return protocol && (protocol.trim() === 'tcp' || protocol.trim() === 'udp');
  }

  addFieldValidator(ctrl: AbstractControl, validator: ValidatorFn) {
    if (!ctrl.validator) {
      ctrl.setValidators([validator]);
    } else {
      ctrl.setValidators([ctrl.validator, validator]);
    }
  }

  isProtocolFieldValid(): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      let val: string = control.value;
      if (!val || !val.trim()) {
        return null;
      }
      val = val.trim();
      if (val === 'tcp' || val === 'udp' || val === 'icmp' || val === 'any') {
        return null;
      }
      return {
        fieldProtocol: {
          required: false,
          message: 'Invalid Protocol. Only tcp, udp, icmp and any are allowed.'
        }
      };
    };
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

  isTimeoutValid(fieldName: string): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      const val: string = control.value;
      if (!val) {
        return null;
      }
      if (!Utility.isTimeoutValid(val)) {
        return {
          [fieldName]: {
            required: false,
            message: 'Invalid time out value. Only h, m, s, ms, us, and ns are allowed'
          }
        };
      }
      return null;
    };
  }
}

