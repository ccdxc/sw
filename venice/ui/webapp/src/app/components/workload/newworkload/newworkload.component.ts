import { Component, OnInit, ViewEncapsulation, AfterViewInit, Input } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Animations } from '@app/animations';
import { IWorkloadWorkload, WorkloadWorkload, WorkloadWorkloadIntfSpec } from '@sdk/v1/models/generated/workload';
import { WorkloadService } from '@app/services/generated/workload.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { FormArray, ValidatorFn } from '@angular/forms';
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
    // Add one interface if it doesn't already have one
    const interfaces = this.newObject.$formGroup.get(['spec', 'interfaces']) as FormArray;
    if (interfaces.length === 0) {
      this.addInterface();
    }

    this.newObject.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
      this.isNewPolicyNameValid(this.existingObjects) ]);
  }

  isNewPolicyNameValid(existingObjects: IWorkloadWorkload[]): ValidatorFn {
    // checks if name field is valid
    return Utility.isModelNameUniqueValidator(existingObjects, 'newWorkload-name');
  }

  addInterface() {
    const interfaces = this.newObject.$formGroup.get(['spec', 'interfaces']) as FormArray;
    const newInterface = new WorkloadWorkloadIntfSpec().$formGroup;
    newInterface.get('ip-addresses').setValidators(IPUtility.isValidIPListValidator);
    interfaces.insert(interfaces.length, newInterface);
  }

  removeInterface(index: number) {
    const interfaces = this.newObject.$formGroup.get(['spec', 'interfaces']) as FormArray;
    if (interfaces.length > 1) {
      interfaces.removeAt(index);
    }
  }

  isValidIP(ip: string) {
    return IPUtility.isValidIP(ip);
  }

  // Empty Hook
  isFormValid() {
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      return false;
    }
    return this.newObject.$formGroup.valid;
  }


  setToolbar() {
    const currToolbar = this.controllerService.getToolbarData();
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary global-button-padding',
        text: 'CREATE WORKLOAD',
        callback: () => { this.saveObject(); },
        computeClass: () => this.computeButtonClass()
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
}
