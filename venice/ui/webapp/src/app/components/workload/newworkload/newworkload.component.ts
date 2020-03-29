import { Component, OnInit, ViewEncapsulation, AfterViewInit, Input } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Animations } from '@app/animations';
import { IWorkloadWorkload, WorkloadWorkload, WorkloadWorkloadIntfSpec } from '@sdk/v1/models/generated/workload';
import { WorkloadService } from '@app/services/generated/workload.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { FormArray, ValidatorFn, AbstractControl, Validators, ValidationErrors, FormGroup } from '@angular/forms';
import { SelectItem } from 'primeng/api';
import { IPUtility } from '@app/common/IPUtility';
import { Utility } from '@app/common/Utility';

@Component({
  selector: 'app-newworkload',
  templateUrl: './newworkload.component.html',
  styleUrls: ['./newworkload.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: Animations,
})
export class NewworkloadComponent extends CreationForm<IWorkloadWorkload, WorkloadWorkload> implements OnInit, AfterViewInit {

  // Let workload.component pass in hostOptions
  @Input() hostOptions: SelectItem[] = [];
  @Input() existingObjects: IWorkloadWorkload[] = [];

  IPS_LABEL: string = 'IP Addresses';
  IPS_ERRORMSG: string = 'Invalid IP addresses';
  MACS_LABEL: string = 'MAC Addresses';
  MACS_ERRORMSG: string = 'Invalid MAC addresses. It should be aaaa.bbbb.cccc format.';
  validationMessage: string;
  defaultHostName: string;

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected workloadService: WorkloadService,
  ) {
    super(_controllerService, uiconfigsService, WorkloadWorkload);
  }

  getClassName() {
    return this.constructor.name;
  }

  // Empty Hook
  postNgInit() {
    if (!this.isInline && this.hostOptions.length) {
      this.newObject.$formGroup.get(['spec', 'host-name']).setValue(this.hostOptions[0].value);
    }
    // Add one interface if it doesn't already have one
    const interfaces = this.newObject.$formGroup.get(['spec', 'interfaces']) as FormArray;

    if (interfaces.length > 0) {
      interfaces.controls.forEach((r, i) => {
        r['inEdit'] = true;
      });
    }

    if (interfaces.length === 0) {
      this.addInterface();
    }



    this.newObject.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
      this.isNewPolicyNameValid(this.existingObjects)]);
  }

  isNewPolicyNameValid(existingObjects: IWorkloadWorkload[]): ValidatorFn {
    // checks if name field is valid
    return Utility.isModelNameUniqueValidator(existingObjects, 'newWorkload-name');
  }

  addInterface() {
    const interfaces = this.newObject.$formGroup.get(['spec', 'interfaces']) as FormArray;
    const newInterface = new WorkloadWorkloadIntfSpec().$formGroup;
    newInterface['inEdit'] = false;

    newInterface.get('mac-address').setValidators([
      this.isValidMacAddress()
    ]);
    newInterface.get('external-vlan').setValidators([this.minVLanValueValidator('External VLAN')]);
    newInterface.get('micro-seg-vlan').setValidators([this.minVLanValueValidator('Micro Segement VLAN')]);

    interfaces.insert(interfaces.length, newInterface);
    this.editInterface(interfaces.length - 1);
  }

  editInterface(index) {
    // Collapse any other open rules, and make index rule open
    this.newObject.$formGroup.get(['spec', 'interfaces'])['controls'].forEach((r, i) => {
      if (i === index) {
        r.inEdit = false;
      } else {
        r.inEdit = true;
      }
    });
  }

  orderedListClick(index: number) {
    if (this.newObject.$formGroup.get(['spec', 'interfaces'])['controls'][index].inEdit) {
      this.editInterface(index);
    }
  }

  removeInterface(index: number) {
    const interfaces = this.newObject.$formGroup.get(['spec', 'interfaces']) as FormArray;
    if (interfaces.length > 1) {
      interfaces.removeAt(index);
    }
  }

  isValidIP(ip: string) {
    return IPUtility.isValidIPWithOptionalMask(ip);

  }

  isValidMacAddress(): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      const macAddress = control.value;
      if (!macAddress) {
        return null;
      }
      if (!Utility.MACADDRESS_REGEX.test(macAddress)) {
        return {
          objectName: {
            required: true,
            message: this.MACS_ERRORMSG
          }
        };
      }
      return null;
    };
  }

  minVLanValueValidator(vlanName: string): ValidatorFn {
    const min: number = vlanName === 'External VLAN' ? 0 : 1;
    return (control: AbstractControl): ValidationErrors | null => {
      const actual = Number(control.value);
      if (actual < min || actual > 4095) {
        return {
          objectName: {
            required: true,
            message: actual + ' is not a valid ' + vlanName + ' number'
          }
        };
      }
      return null;
    };
  }


  // Empty Hook
  isFormValid() {
    this.validationMessage = null;
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      this.validationMessage = 'Error: Workload name is required.';
      return false;
    }
    for (let i = 0; i < this.newObject.$formGroup.get(['spec', 'interfaces']).value.length; i++) {

      const fieldValue = this.newObject.$formGroup.get(['spec', 'interfaces'])['controls'][i];
      if ((fieldValue.controls['mac-address']).value !== null) {
        if (!(fieldValue.controls['mac-address']).valid) {
          this.validationMessage =
            'Error: Interface ' + (i + 1) + ' source Mac addresses are invalid.';
          return false;
        }
      }
      if ((fieldValue.controls['ip-addresses']).value.length) {
        if (!(fieldValue.controls['ip-addresses']).valid) {
          this.validationMessage =
            'Error: Interface ' + (i + 1) + ' destination IP addresses are invalid.';
          return false;
        }
      }
      if ((fieldValue.controls['external-vlan']).value !== null) {
        if (!(fieldValue.controls['external-vlan']).valid) {
          this.validationMessage =
            'Error: Interface ' + (i + 1) + ' external-vlan is invalid.';
          return false;
        }
      }
      if ((fieldValue.controls['micro-seg-vlan']).value !== null) {
        if (!(fieldValue.controls['micro-seg-vlan']).valid) {
          this.validationMessage =
            'Error: Interface ' + (i + 1) + ' micro-seg-vlan is invalid.';
          return false;
        }
      }
    }
    if (!this.areInterfaceValid()) {
      this.validationMessage = 'Error: At least one interface must be specified';
      return false;
    }

    return true;
  }


  setToolbar() {
    const currToolbar = this.controllerService.getToolbarData();
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary global-button-padding',
        text: 'CREATE WORKLOAD',
        callback: () => { this.saveObject(); },
        computeClass: () => this.computeButtonClass(),
        genTooltip: () => this.getTooltip(),
      },
      {
        cssClass: 'global-button-neutral global-button-padding',
        text: 'CANCEL',
        callback: () => { this.cancelObject(); }
      },
    ];

    this._controllerService.setToolbarData(currToolbar);
  }

  getTooltip(): string {
    this.isFormValid();
    return Utility.isEmpty(this.validationMessage) ? 'Ready to save new workload' : this.validationMessage;
  }


  createObject(object: IWorkloadWorkload) {
    return this.workloadService.AddWorkload(object);
  }

  updateObject(newObject: IWorkloadWorkload, oldObject: IWorkloadWorkload) {
    return this.workloadService.UpdateWorkload(oldObject.meta.name, newObject, null, oldObject);
  }

  generateCreateSuccessMsg(object: IWorkloadWorkload) {
    return 'Created workload ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: IWorkloadWorkload) {
    return 'Updated workload ' + object.meta.name;
  }

  getInputFieldValue(index: number, field: string): string {
    return this.newObject.$formGroup.get(['spec', 'interfaces']).value[index][field];

  }


  formControls(): AbstractControl[] {
    return (this.newObject.$formGroup.get(['spec', 'interfaces']) as FormArray).controls;
  }

  formControlName(index: number, field: string): AbstractControl {
    const formControl = this.formControls();
    return (formControl[index] as FormGroup).controls[field];
  }

  areInterfaceValid(): boolean {
    return this.newObject.$formGroup.get(['spec', 'interfaces']).valid;
  }

}
