import { Component, OnInit, Input, ViewEncapsulation, ViewChild } from '@angular/core';
import { Animations } from '@app/animations';
import { SelectItem } from 'primeng/primeng';
import { Utility } from '@app/common/Utility';
import { MonitoringSyslogExport, MonitoringExternalCred, IMonitoringSyslogExport, MonitoringExportConfig, MonitoringSyslogExportConfig } from '@sdk/v1/models/generated/monitoring';
import { FormArray, FormGroup, AbstractControl, ValidatorFn, ValidationErrors } from '@angular/forms';
import { BaseComponent } from '@app/components/base/base.component';
import { ControllerService } from '@app/services/controller.service';

@Component({
  selector: 'app-syslog',
  templateUrl: './syslog.component.html',
  styleUrls: ['./syslog.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class SyslogComponent extends BaseComponent implements OnInit {
  @Input() syslogExport: IMonitoringSyslogExport;
  @Input() showSyslogOptions: boolean = true;
  @Input() targetTransport: String = '<protocol>/<port>';
  @Input() formatOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringSyslogExport.propInfo['format'].enum);
  @Input() maxTargets: number;
  @Input() customizedValidator: () => boolean = null;
  @Input() syslogRequiredOption: boolean;
  @Input() syslogFieldsetWidth: boolean;

  syslogServerForm: FormGroup;

  syslogOverrideOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringSyslogExportConfig.propInfo['facility-override'].enum);

  syslogCredentialOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringExternalCred.propInfo['auth-type'].enum);

  targets: any;

  constructor(protected _controllerService: ControllerService) {
    super(_controllerService);
  }

  ngOnInit() {
    let syslogExport;
    if (this.syslogExport != null) {
      syslogExport = new MonitoringSyslogExport(this.syslogExport);
    } else {
      syslogExport = new MonitoringSyslogExport();
    }
    this.syslogServerForm = syslogExport.$formGroup;
    const targets: any = this.syslogServerForm.get('targets');
    if (targets.controls.length === 0) {
      this.addTarget();
    }
    this.targets = (<any>this.syslogServerForm.get(['targets'])).controls;
  }

  getValues(): IMonitoringSyslogExport {
    // remove syslog configs if they are off
    if (this.showSyslogOptions) {
      return this.syslogServerForm.value;
    } else {
      const _ = Utility.getLodash();
      // Cloning so that we don't change the form group object
      const formValues = _.cloneDeep(this.syslogServerForm.value);
      delete formValues.config;
      return formValues;
    }
  }

  getSelectedCredentialMethod(index: number): string {
    return this.syslogServerForm.get(['targets', index, 'credentials', 'auth-type']).value;
  }

  addTarget() {
    const targets = this.syslogServerForm.get('targets') as FormArray;
    targets.insert(0, new MonitoringExportConfig().$formGroup);
    const targetArr = (<any>this.syslogServerForm.get(['targets'])).controls;
    for (let i = 0; i < targetArr.length; i++) {
      this.syslogServerForm.get(['targets', i]).get('transport').setValidators([
        this.isTransportFieldValue(this.syslogServerForm.get(['targets', i]).get('transport'))]);
    }
  }

  removeTarget(index) {
    const targets = this.syslogServerForm.get('targets') as FormArray;
    if (targets.length > 1) {
      targets.removeAt(index);
    }
  }

  /**
   * validate inputs
   * If there is @input va
   */
  isValid(): boolean {
    if (!this.customizedValidator) {
      return this.allTargetsEmpty();
    } else {
      return this.customizedValidator();
    }
  }

  getNumberOfEmptyTarget(): number {
    const targets = this.syslogServerForm.get('targets').value;  // return an array
    let countEmptyRule: number = 0;
    for (let i = 0; i < targets.length; i++) {
      const target = targets[i];
      // content is like
      // "{"destination":null,"transport":null,"credentials":{"auth-type":"none","username":null,"password":null,"bearer-token":null,"cert-data":null,"key-data":null,"ca-data":null}}"
      const content = Utility.TrimDefaultsAndEmptyFields(target);
      if (Utility.isEmpty(content.destination) || Utility.isEmpty(content.transport)) {
        countEmptyRule += 1;
      }
    }
    return countEmptyRule;
  }

  allTargetsEmpty(): boolean {
    const targets = this.syslogServerForm.get('targets').value;  // return an array
    const countEmptyRule = this.getNumberOfEmptyTarget();
    return (countEmptyRule === targets.length);
  }

  atLeastOneTargetFilled(): boolean {
    const targets = this.syslogServerForm.get('targets').value;  // return an array
    const countEmptyRule = this.getNumberOfEmptyTarget();
    return (countEmptyRule < targets.length);
  }
  isTargetFieldEmpty(): boolean {
    const countEmptyRule = this.getNumberOfEmptyTarget();
    if (countEmptyRule > 0) {
      return true;
    }
  }
  isFieldEmptySysLog(field: boolean): boolean {
    return Utility.isEmpty(field);
  }
  isSyLogFormValid(): object {
    const returnObject = {
      errorMessage: '',
      valid: true
    };
    if (this.isTargetFieldEmpty()) {
      returnObject.errorMessage = 'Error: Target field is required.';
      returnObject.valid = false;
      return returnObject;
    }
    if (!this.syslogServerForm.get(['targets']).valid) {
      returnObject.errorMessage = 'Error:Transport Field is Invalid.';
      returnObject.valid = false;
      return returnObject;
    }
    return returnObject;
  }
  isTransportFieldValue(transportVal): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      const val: string = control.value;
      let protocolVal: string;
      let port: string;
      if (val) {
        protocolVal = val.split('/')[0];
        port = val.split('/')[1];
      }
      if (!val) {
        return null;
      }
      if ((protocolVal.toLowerCase() === 'tcp' || protocolVal.toLowerCase() === 'udp') && (val.includes('/')) && (Utility.isPortValid(port))) {
        return null;
      }
      return {
        fieldProtocol: {
          required: false,
          message: 'Invalid Protocol. Only tcp and udp are allowed,port should be a number between 1 to 65536.'
        }
      };
    };
  }
}
