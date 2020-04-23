import { AfterViewInit, ChangeDetectionStrategy, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Animations } from '@app/animations';
import { IWorkloadWorkload, WorkloadWorkload, WorkloadWorkloadIntfSpec } from '@sdk/v1/models/generated/workload';
import { WorkloadService } from '@app/services/generated/workload.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { FormArray, ValidatorFn, AbstractControl, Validators, ValidationErrors, FormGroup } from '@angular/forms';
import { minValueValidator, maxValueValidator } from '@sdk/v1/utils/validators';
import { SelectItem } from 'primeng/api';
import { IPUtility } from '@app/common/IPUtility';
import { Utility } from '@app/common/Utility';

@Component({
  selector: 'app-newworkload',
  templateUrl: './newworkload.component.html',
  styleUrls: ['./newworkload.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: Animations,
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class NewworkloadComponent extends CreationForm<IWorkloadWorkload, WorkloadWorkload> implements OnInit, AfterViewInit {

  // Let workload.component pass in hostOptions
  @Input() hostOptions: SelectItem[] = [];
  @Input() existingObjects: IWorkloadWorkload[] = [];

  @Output() editFormClose: EventEmitter<any> = new EventEmitter<any>();

  IPS_LABEL: string = 'IP Addresses';
  IPS_ERRORMSG: string = 'Invalid IP addresses';
  IPS_TOOLTIP: string = 'Type in ip address and hit enter or space key to add more.';
  MACS_LABEL: string = 'MAC Addresses';
  MACS_ERRORMSG: string = 'Invalid MAC addresses. It should be aaaa.bbbb.cccc format.';
  validationMessage: string;

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
      this.hostOptions.push({label: '', value: null});
    }
    // Add one interface if it doesn't already have one
    const interfaces = this.newObject.$formGroup.get(['spec', 'interfaces']) as FormArray;

    if (interfaces.length === 0) {
      this.addInterface();
    } else {
      interfaces.controls.forEach((form: FormGroup) => {
        if (!form.get(['external-vlan']).value) {
          form.get(['external-vlan']).setValue(0);
        }
      });
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

    newInterface.get('mac-address').setValidators([
      this.isValidMacAddress()
    ]);
    newInterface.get('external-vlan').setValidators([minValueValidator(0), maxValueValidator(4095)]);
    newInterface.get('micro-seg-vlan').setValidators([minValueValidator(1), maxValueValidator(4095)]);

    interfaces.push(newInterface);
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

  // Empty Hook
  isFormValid() {
    this.validationMessage = null;
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      return false;
    }
    if (Utility.isEmpty(this.newObject.$formGroup.get(['spec', 'host-name']).value)) {
      this.validationMessage = 'Error: Host is required.';
      return false;
    }
    const arr: FormArray = this.newObject.$formGroup.get(['spec', 'interfaces']) as FormArray;
    for (let i = 0; i < arr.length; i++) {
      const fieldValue: FormGroup = arr['controls'][i] as FormGroup;
      if ((fieldValue.controls['mac-address']).value !== null) {
        if (!(fieldValue.controls['mac-address']).valid) {
          this.validationMessage =
            'Error: Interface ' + (i + 1) + ' source MAC address is invalid.';
          return false;
        }
      } else {
        this.validationMessage =
          'Error: Interface ' + (i + 1) + ' source MAC address is empty.';
        return false;
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
      } else {
        this.validationMessage =
          'Error: Interface ' + (i + 1) + ' external-vlan is empty.';
        return false;
      }
      if ((fieldValue.controls['micro-seg-vlan']).value !== null) {
        if (!(fieldValue.controls['micro-seg-vlan']).valid) {
          this.validationMessage =
            'Error: Interface ' + (i + 1) + ' micro-seg-vlan is invalid.';
          return false;
        }
      } else {
        this.validationMessage =
          'Error: Interface ' + (i + 1) + '  micro-seg-vlan is empty.';
        return false;
      }
    }
    if (!this.newObject.$formGroup.valid) {
      this.validationMessage = 'Error: form is not valid';
      return false;
    }

    return true;
  }

  oneInterfaceIsValid(): boolean {
    const array: FormArray = this.newObject.$formGroup.get(['spec', 'interfaces']) as FormArray;
    for (let i = 0; i < array.controls.length; i++) {
      if (this.isInterfaceValid(array.controls[i] as FormGroup)) {
        return true;
      }
    }
    return false;
  }

  isInterfaceValid(form: FormGroup) {
    const fields = [
      form.get(['mac-address']),
      form.get(['external-vlan']),
      form.get(['micro-seg-vlan'])
    ];
    for (let i = 0; i < fields.length; i++) {
      if (Utility.isEmpty(fields[i].value) || !fields[i].valid) {
        return false;
      }
    }
    const field = form.get(['ip-addresses']);
    return field.valid;
  }

  getTooltip(): string {
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      return 'Error: Workload name is required.';
    }
    return Utility.isEmpty(this.validationMessage) ? 'Ready to save new workload' : this.validationMessage;
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

  editSaveObject() {
    this.saveObject();
    this.editFormClose.emit();
  }

  editCancelObject() {
    this.cancelObject();
    this.editFormClose.emit();
  }
}
