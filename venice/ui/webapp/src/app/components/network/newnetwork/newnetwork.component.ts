import { AfterViewInit, ChangeDetectionStrategy, ChangeDetectorRef, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Animations } from '@app/animations';
import { NetworkNetwork, INetworkNetwork, NetworkOrchestratorInfo } from '@sdk/v1/models/generated/network';
import { NetworkService } from '@app/services/generated/network.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { FormArray, ValidatorFn, FormGroup, FormControl, AbstractControl, ValidationErrors } from '@angular/forms';
import { Utility } from '@app/common/Utility';
import { OrchestrationOrchestrator } from '@sdk/v1/models/generated/orchestration';
import { SelectItem } from 'primeng/api';
import { minValueValidator, maxValueValidator, CustomFormControl } from '@sdk/v1/utils/validators';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';

@Component({
  selector: 'app-newnetwork',
  templateUrl: './newnetwork.component.html',
  styleUrls: ['./newnetwork.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: Animations,
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class NewnetworkComponent extends CreationForm<INetworkNetwork, NetworkNetwork> implements OnInit, AfterViewInit {
  @Input() vcenterOptions: SelectItem[] = [];
  @Input() vcenters: ReadonlyArray<OrchestrationOrchestrator> = [];
  @Input() existingObjects: INetworkNetwork[] = [];

  @Output() editFormClose: EventEmitter<any> = new EventEmitter<any>();

  datacenterNames: SelectItem[] = [];

  ALL_DATACENTERS: string = 'all_namespaces';
  EACH_DATACENTER: string = 'each';

  chooseOptions: SelectItem[] = [
    {label: 'Choose All Datacenters', value: this.ALL_DATACENTERS},
    {label: 'Choose Individual Datacenters', value: this.EACH_DATACENTER}
  ];

  existingObjectVlanId: number = -1;

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected networkService: NetworkService,
    private cdr: ChangeDetectorRef
  ) {
    super(_controllerService, uiconfigsService, NetworkNetwork);
  }

  getClassName() {
    return this.constructor.name;
  }

  // merge all datacenters of the same vcenter together
  loadExistingObject(data: NetworkNetwork) {
    if (data.spec && data.spec.orchestrators && data.spec.orchestrators.length > 0) {
      const map: {'vCenter': string, 'dataCenters': string[]} = {} as any;
      data.spec.orchestrators.forEach((value: NetworkOrchestratorInfo) => {
        if (!map[value['orchestrator-name']]) {
          map[value['orchestrator-name']] = [value.namespace];
        } else {
          map[value['orchestrator-name']].push(value.namespace);
        }
      });
      data = Utility.getLodash().cloneDeep(data);
      data.spec.orchestrators = [];
      for (const key of Object.keys(map)) {
        data.spec.orchestrators.push({
          '_ui': {},
          'orchestrator-name': key,
          'namespace': map[key]
        } as NetworkOrchestratorInfo);
      }
    }
    return data;
  }

  postNgInit() {
    if (!this.isInline) {
      this.newObject.$formGroup.get(['meta', 'name']).setValidators([
        this.newObject.$formGroup.get(['meta', 'name']).validator,
        this.isNewPolicyNameValid(this.existingObjects)]);
    } else {
      this.newObject.$formGroup.get(['meta', 'name']).setValidators(null);
      // vs-1397, 0 is default vlan id
      if (!this.newObject.$formGroup.get(['spec', 'vlan-id']).value) {
        this.newObject.$formGroup.get(['spec', 'vlan-id']).setValue(0);
      }

      const orchestrators: FormArray =
        this.newObject.$formGroup.get(['spec', 'orchestrators']) as FormArray;
      if (orchestrators && orchestrators.length > 0) {
        orchestrators.controls.forEach((orchestrator: FormGroup) => {
          orchestrator.addControl('datacenterChoice', CustomFormControl(new FormControl('', null), {}));
          let datacenterChoice = this.ALL_DATACENTERS;
          if (this.isVcenterManagedAllDCsByName(orchestrator.value['orchestrator-name'])) {
            const namespace = orchestrator.value['namespace'];
            if (namespace && namespace.length > 0 && namespace[0] !== this.ALL_DATACENTERS) {
              datacenterChoice = this.EACH_DATACENTER;
            }
          }
          orchestrator.get('datacenterChoice').setValue(datacenterChoice);
          if (orchestrator.value && orchestrator.value['orchestrator-name']) {
            const orchestratorObj = orchestrator as any;
            orchestratorObj.datacenterOptions =
              this.generateDCNamesOptions(orchestrator.value['orchestrator-name']);
          }
        });
      }
    }
    const ctrl: AbstractControl = this.newObject.$formGroup.get(['spec', 'vlan-id']);
    if (this.isInline) {
      ctrl.disable();
      this.existingObjectVlanId = ctrl.value;
    } else {
      this.addFieldValidator(ctrl, this.isVlanAlreadyUsed(this.existingObjects));
      this.addFieldValidator(ctrl, minValueValidator(0));
    }

    // Add one collectors if it doesn't already have one
    const collectors = this.newObject.$formGroup.get(['spec', 'orchestrators']) as FormArray;
    if (collectors.length === 0) {
      this.addOrchestrator();
    }
  }

  isNewPolicyNameValid(existingObjects: INetworkNetwork[]): ValidatorFn {
    // checks if name field is valid
    return Utility.isModelNameUniqueValidator(existingObjects, 'newNetwork-name');
  }

  isVlanAlreadyUsed(existingObjects: INetworkNetwork[]): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      if (!control.value && control.value !== 0) {
        return null;
      }
      const networkObj: INetworkNetwork = this.existingObjects.find((item: INetworkNetwork) =>
        item.spec['vlan-id'] === control.value || (control.value === 0 && !item.spec['vlan-id']));
      if (networkObj) {
        return {
          objectname: {
            required: true,
            message: 'Network vlanID must be unique, already used by ' + networkObj.meta.name
          }
        };
      }
      return null;
    };
  }

  addOrchestrator() {
    const orchestrators = this.newObject.$formGroup.get(['spec', 'orchestrators']) as FormArray;
    const newOrchestrator = new NetworkOrchestratorInfo().$formGroup;
    newOrchestrator.addControl('datacenterChoice', CustomFormControl(new FormControl('', null), {}));
    newOrchestrator.get('datacenterChoice').setValue(this.ALL_DATACENTERS);
    orchestrators.insert(orchestrators.length, newOrchestrator);
    this.newObject.$formGroup.markAsDirty();
  }

  removeOrchestrator(index: number) {
    const orchestrators = this.newObject.$formGroup.get(['spec', 'orchestrators']) as FormArray;
    if (orchestrators.length > 1) {
      orchestrators.removeAt(index);
      this.newObject.$formGroup.markAsDirty();
    }
  }

  // Empty Hook
  isFormValid() {
    if (!this.isInline) {
      if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
        this.submitButtonTooltip = 'Error: Name field is empty.';
        return false;
      }
      if (this.newObject.$formGroup.get(['meta', 'name']).invalid)  {
        this.submitButtonTooltip = 'Error: Name field is invalid.';
        return false;
      }
      if (Utility.isEmpty(this.newObject.$formGroup.get(['spec', 'vlan-id']).value)) {
        this.submitButtonTooltip = 'Error: VLAN is required.';
        return false;
      }

      if (!this.newObject.$formGroup.get(['spec', 'vlan-id']).valid) {
        this.submitButtonTooltip = 'Error: Invalid VLAN';
        return false;
      }
    }

    const orchestrators = this.controlAsFormArray(
      this.newObject.$formGroup.get(['spec', 'orchestrators'])).controls;
    for (let i = 0; i < orchestrators.length; i++) {
      const orchestrator = orchestrators[i];
      if (!Utility.isEmpty(orchestrator.get('orchestrator-name').value)) {
        const vcenterName = orchestrator.get('orchestrator-name').value;
        if ((this.isVcenterManagedAllDCsByName(vcenterName) &&
            orchestrator.get('datacenterChoice').value !== this.ALL_DATACENTERS) ||
            !this.isVcenterManagedAllDCsByName(vcenterName)) {
          if (Utility.isEmpty(orchestrator.get(['namespace']).value)) {
            this.submitButtonTooltip = 'Error: Datacenter name is required.';
            return false;
          }
        }
      }
    }

    this.submitButtonTooltip = 'Ready to submit';
    return true;
  }

  getObjectValues(): INetworkNetwork {
    const currValue: INetworkNetwork =  this.newObject.getFormGroupValues();
    // when in editting, vlan-id box is disabled, need manually set vlan-id
    // before submitting.
    if (this.isInline) {
      currValue.spec['vlan-id'] = this.existingObjectVlanId;
    }
    const orchestrators = [];
    if (currValue.spec.orchestrators && currValue.spec.orchestrators.length > 0) {
      currValue.spec.orchestrators.forEach((each) => {
        const item: any = each;
        if (item['orchestrator-name']) {
          const datacenterChoice = item.datacenterChoice;
          if (item.namespace && item.namespace.length > 0) {
            const namespaceArr = item.namespace as any;
            namespaceArr.forEach((namespace: string) => {
              orchestrators.push({
                'orchestrator-name': item['orchestrator-name'],
                namespace
              });
            });
          } else if (datacenterChoice === this.ALL_DATACENTERS) {
            orchestrators.push({
              'orchestrator-name': item['orchestrator-name'],
              namespace: this.ALL_DATACENTERS
            });
          }
          delete item.datacenterChoice;
        }
      });
    }
    currValue.spec.orchestrators = orchestrators;
    return currValue;
  }

  setToolbar() {
    if (!this.isInline && this.uiconfigsService.isAuthorized(UIRolePermissions.networknetwork_create)) {
      const currToolbar = this.controllerService.getToolbarData();
      currToolbar.buttons = [
        {
          cssClass: 'global-button-primary global-button-padding',
          text: 'CREATE NETWORK',
          callback: () => { this.saveObject(); },
          computeClass: () => this.computeFormSubmitButtonClass(),
          genTooltip: () => this.getSubmitButtonToolTip(),
        },
        {
          cssClass: 'global-button-neutral global-button-padding',
          text: 'CANCEL',
          callback: () => { this.cancelObject(); }
        },
      ];

      this._controllerService.setToolbarData(currToolbar);
    }
  }

  editSaveObject() {
    this.saveObject();
    this.editFormClose.emit();
  }

  editCancelObject() {
    this.cancelObject();
    this.editFormClose.emit();
  }

  onVcenterChange(orchestrator: FormGroup) {
    orchestrator.get(['namespace']).setValue(null);
    const orchestratorObj = orchestrator as any;
    orchestratorObj.datacenterOptions =
      this.generateDCNamesOptions(orchestrator.value['orchestrator-name']);
    this.cdr.detectChanges();
  }

  onDatacenterChoiceChange(event: any, orchestrator: FormGroup) {
    orchestrator.get('namespace').setValue([]);
  }

  showDatacenterChoices(orchestrator: FormGroup) {
    const vcenterName = orchestrator.value['orchestrator-name'];
    return this.isVcenterManagedAllDCsByName(vcenterName);
  }

  showDatacenterNames(orchestrator: FormGroup) {
    const vcenterName = orchestrator.value['orchestrator-name'];
    return !this.isVcenterManagedAllDCsByName(vcenterName) ||
        orchestrator.value['datacenterChoice'] !== this.ALL_DATACENTERS;
  }

  generateDCNamesOptions(vCenter: string): SelectItem[] {
    if (vCenter && this.vcenters && this.vcenters.length > 0) {
      const vcenterObj: OrchestrationOrchestrator = this.vcenters.find(
        item => item && item.meta && item.meta.name === vCenter
      );
      if (!this.isVcenterManagedAllDCs(vcenterObj)) {
        const options: SelectItem[] = vcenterObj.spec['manage-namespaces'].map(item => {
          return {
            label: item,
            value: item
          };
        });
        return options;
      }
      const discoveredDatacenters: string[] = vcenterObj.status['discovered-namespaces'];
      if (discoveredDatacenters && discoveredDatacenters.length > 0) {
        const options: SelectItem[] =  discoveredDatacenters.map(item => {
          return {
            label: item,
            value: item
          };
        });
        return options;
      }
    }
    return [];
  }

  getVencetrObjectByName(vCenterName: string): OrchestrationOrchestrator {
    if (vCenterName && this.vcenters && this.vcenters.length > 0) {
      return this.vcenters.find(
        item => item && item.meta && item.meta.name === vCenterName);
    }
    return null;
  }

  isVcenterManagedAllDCs(vcenterObj: OrchestrationOrchestrator): boolean {
    if (vcenterObj && vcenterObj.spec && vcenterObj.spec['manage-namespaces'] &&
        vcenterObj.spec['manage-namespaces'].length > 0 &&
        vcenterObj.spec['manage-namespaces'][0] === 'all_namespaces') {
      return true;
    }
    return false;
  }

  isVcenterManagedAllDCsByName(vCenterName: string): boolean {
    return this.isVcenterManagedAllDCs(this.getVencetrObjectByName(vCenterName));
  }

  createObject(object: INetworkNetwork) {
    return this.networkService.AddNetwork(object);
  }

  updateObject(newObject: INetworkNetwork, oldObject: INetworkNetwork) {
    return this.networkService.UpdateNetwork(oldObject.meta.name, newObject, null, oldObject);
  }

  generateCreateSuccessMsg(object: INetworkNetwork) {
    return 'Created network ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: INetworkNetwork) {
    return 'Updated network ' + object.meta.name;
  }

}
