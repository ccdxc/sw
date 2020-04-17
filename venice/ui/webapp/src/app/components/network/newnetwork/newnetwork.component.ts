import { Component, OnInit, ViewEncapsulation, AfterViewInit, Input, ChangeDetectionStrategy, ChangeDetectorRef, ViewChild } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Animations } from '@app/animations';
import { NetworkNetwork, INetworkNetwork, NetworkOrchestratorInfo, INetworkOrchestratorInfo } from '@sdk/v1/models/generated/network';
import { NetworkService } from '@app/services/generated/network.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { FormArray, ValidatorFn, AbstractControl, FormGroup } from '@angular/forms';
import { Utility } from '@app/common/Utility';
import { OrchestrationOrchestrator } from '@sdk/v1/models/generated/orchestration';
import { SelectItem } from 'primeng/api';
import { minValueValidator, maxValueValidator } from '@sdk/v1/utils/validators';
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

  datacenterNames: SelectItem[] = [];

  createButtonTooltip: string = 'Ready to submit';

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

  // Empty Hook
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
          if (orchestrator.value && orchestrator.value['orchestrator-name']) {
            const orchestratorObj = orchestrator as any;
            orchestratorObj.datacenterOptions =
              this.generateDCNamesOptions(orchestrator.value['orchestrator-name']);
          }
        });
      }

    }

    this.newObject.$formGroup.get(['spec', 'vlan-id']).setValidators(
      [minValueValidator(0), maxValueValidator(65536)]);

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

  addOrchestrator() {
    const orchestrators = this.newObject.$formGroup.get(['spec', 'orchestrators']) as FormArray;
    const newOrchestrator = new NetworkOrchestratorInfo().$formGroup;
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
    if (Utility.isEmpty(this.newObject.$formGroup.get(['spec', 'vlan-id']).value)) {
      this.createButtonTooltip = 'Error: VLAN is required.';
      return false;
    }

    if (!this.newObject.$formGroup.get(['spec', 'vlan-id']).valid) {
      this.createButtonTooltip = 'Error: Invalid VLAN';
      return false;
    }

    const orchestrators = this.controlAsFormArray(
      this.newObject.$formGroup.get(['spec', 'orchestrators'])).controls;
    for (let i = 0; i < orchestrators.length; i++) {
      const orchestrator = orchestrators[i];
      if (Utility.isEmpty(orchestrator.get(['orchestrator-name']).value)) {
        this.createButtonTooltip = 'Error: VCenter name is required.';
        return false;
      }
      if (Utility.isEmpty(orchestrator.get(['namespace']).value)) {
        this.createButtonTooltip = 'Error: Datacenter name is required.';
        return false;
      }
    }

    this.createButtonTooltip = 'Ready to submit';
    return true;
  }

  getTooltip(): string {
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      return 'Error: Name field is empty.';
    }
    if (this.newObject.$formGroup.get(['meta', 'name']).invalid)  {
      return 'Error: Name field is invalid.';
    }
    return this.createButtonTooltip;
  }

  getObjectValues(): INetworkNetwork {
    const currValue: INetworkNetwork =  this.newObject.getFormGroupValues();
    const orchestrators = [];
    if (currValue.spec.orchestrators && currValue.spec.orchestrators.length > 0) {
      currValue.spec.orchestrators.forEach(item => {
        if (item.namespace && item.namespace.length > 0) {
          const namespaceArr = item.namespace as any;
          namespaceArr.forEach((namespace: string) => {
            orchestrators.push({
              'orchestrator-name': item['orchestrator-name'],
              namespace
            });
          });
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
  }

  onVcenterChange(orchestrator: FormGroup) {
    orchestrator.get(['namespace']).setValue(null);
    const orchestratorObj = orchestrator as any;
    orchestratorObj.datacenterOptions =
      this.generateDCNamesOptions(orchestrator.value['orchestrator-name']);
    this.cdr.detectChanges();
  }

  generateDCNamesOptions(vCenter: string): SelectItem[] {
    if (vCenter && this.vcenters && this.vcenters.length > 0) {
      const vcenterObj: OrchestrationOrchestrator = this.vcenters.find(
        item => item && item.meta && item.meta.name === vCenter
      );
      if (vcenterObj && vcenterObj.spec && vcenterObj.spec['manage-namespaces'] &&
          vcenterObj.spec['manage-namespaces'].length > 0) {
        if (vcenterObj.spec['manage-namespaces'].length > 1 ||
            vcenterObj.spec['manage-namespaces'][0] !== 'all_namespaces') {
          const options: SelectItem[] = vcenterObj.spec['manage-namespaces'].map(item => {
            return {
              label: item,
              value: item
            };
          });
          return options;
        } else {
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
      }
    }
    return [];
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
