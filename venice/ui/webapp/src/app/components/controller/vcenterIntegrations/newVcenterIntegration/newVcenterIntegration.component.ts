import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation, OnDestroy, ViewChild, ChangeDetectorRef, ChangeDetectionStrategy } from '@angular/core';
import { Animations } from '@app/animations';
import { ControllerService } from '@app/services/controller.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { OrchestrationService } from '@app/services/generated/orchestration.service';
import { Utility } from '@app/common/Utility';
import { CustomFormControl } from '@sdk/v1/utils/validators';
import { FormGroup, ValidatorFn, AbstractControl, FormControl } from '@angular/forms';
import { IApiStatus, OrchestrationOrchestrator, IOrchestrationOrchestrator,
  IMonitoringExternalCred, MonitoringExternalCred_auth_type } from '@sdk/v1/models/generated/orchestration';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Observable } from 'rxjs';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { SelectItem } from 'primeng/api';

@Component({
  selector: 'app-newvcenter-integration',
  templateUrl: './newVcenterIntegration.component.html',
  styleUrls: ['./newVcenterIntegration.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class NewVcenterIntegrationComponent extends CreationForm<IOrchestrationOrchestrator, OrchestrationOrchestrator> implements OnInit, AfterViewInit, OnDestroy {

  @Input() isInline: boolean = false;
  @Input() existingObjects: IOrchestrationOrchestrator[] = [];

  DCNAMES_TOOLTIP: string = 'Type in datacenter name and hit enter or space key to add more.';
  ALL_DATACENTERS: string = 'all_namespaces';

  credentialTypes = Utility.convertEnumToSelectItem(MonitoringExternalCred_auth_type);
  currentObjCredType: string = 'none';


  chooseOptions: SelectItem[] = [
    {label: 'Manage All Datacenters', value: this.ALL_DATACENTERS},
    {label: 'Manage Individual Datacenters', value: 'each'}
  ];
  pickedOption: String = this.ALL_DATACENTERS;

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected orchestrationService: OrchestrationService
  ) {
    super(_controllerService, uiconfigsService, OrchestrationOrchestrator);
  }

  postNgInit() {
    // for release B, vs-1609, set password as the only credetial method
    this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).setValue(
      MonitoringExternalCred_auth_type['username-password']
    );
    if (!this.isInline) {
      this.newObject.$formGroup.get(['spec', 'credentials', 'disable-server-authentication']).setValue(true);
    }
    this.setValidators(this.newObject);
    if (this.isInline) {
      this.processVcenterIntegration();
    }
  }

  setValidators(newObject: OrchestrationOrchestrator) {
    if (!this.isInline) {
      newObject.$formGroup.get(['meta', 'name']).setValidators([
        this.newObject.$formGroup.get(['meta', 'name']).validator,
        this.isNewVcenterIntegrationNameValid(this.existingObjects)]);
    } else {
      newObject.$formGroup.get(['meta', 'name']).setValidators(null);
      // disable name field
      this.newObject.$formGroup.get(['meta', 'name']).disable();
    }
  }

  processVcenterIntegration() {
    const dataCenters = this.newObject.$formGroup.get(['spec', 'manage-namespaces']).value;
    if (dataCenters) {
      if (dataCenters.length === 1 && dataCenters[0] === this.ALL_DATACENTERS) {
        this.newObject.$formGroup.get(['spec', 'manage-namespaces']).setValue(null);
      } else {
        this.pickedOption = 'each';
      }
    }
    this.currentObjCredType =
      this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value;
    // clear crdentials
    if (this.currentObjCredType === MonitoringExternalCred_auth_type['username-password']) {
      this.newObject.$formGroup.get(['spec', 'credentials', 'password']).setValue('');
    }
    if (this.currentObjCredType === MonitoringExternalCred_auth_type.token) {
      this.newObject.$formGroup.get(['spec', 'credentials', 'bearer-token']).setValue('');
    }
    if (this.currentObjCredType === MonitoringExternalCred_auth_type.certs) {
      this.newObject.$formGroup.get(['spec', 'credentials', 'key-data']).setValue('');
      this.newObject.$formGroup.get(['spec', 'credentials', 'cert-data']).setValue('');
      this.newObject.$formGroup.get(['spec', 'credentials', 'ca-data']).setValue('');
    }
  }

  isFormValid(): boolean {
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      this.submitButtonTooltip = 'Error: Name field is empty.';
      return false;
    }
    if (this.newObject.$formGroup.get(['meta', 'name']).invalid)  {
      this.submitButtonTooltip = 'Error: Name field is invalid.';
      return false;
    }
    if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'uri']))) {
      this.submitButtonTooltip = 'Error: URI is empty.';
      return false;
    }
    if (this.pickedOption !== this.ALL_DATACENTERS && this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'manage-namespaces']))) {
      this.submitButtonTooltip = 'Error: Datacenter names are empty.';
      return false;
    }
    if (!this.isInline) {
      if (this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
          === MonitoringExternalCred_auth_type['username-password']) {
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'username']))) {
          this.submitButtonTooltip = 'Error: Username is empty.';
          return false;
        }
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'password']))) {
          this.submitButtonTooltip = 'Error: Password is empty.';
          return false;
        }
      }
      if (this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
          === MonitoringExternalCred_auth_type.token) {
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'bearer-token']))) {
          this.submitButtonTooltip = 'Error: Bearer Token is empty.';
          return false;
        }
      }
      if (this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
          === MonitoringExternalCred_auth_type.certs) {
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'key-data']))) {
          this.submitButtonTooltip = 'Error: Private key is empty.';
          return false;
        }
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'cert-data']))) {
          this.submitButtonTooltip = 'Error: Cert file is empty.';
          return false;
        }
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'ca-data']))) {
          this.submitButtonTooltip = 'Error: CA bundle file is empty.';
          return false;
        }
      }
    } else {
      if (this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
          === MonitoringExternalCred_auth_type['username-password']) {
        if (this.currentObjCredType !== MonitoringExternalCred_auth_type['username-password']) {
          if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'username']))) {
            this.submitButtonTooltip = 'Error: Username is empty.';
            return false;
          }
          if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'password']))) {
            this.submitButtonTooltip = 'Error: Password is empty.';
            return false;
          }
        } else {
          if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'username'])) &&
              !this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'password']))) {
            this.submitButtonTooltip = 'Error: Username is empty.';
            return false;
          }
        }
      }
    }
    if (this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
          === MonitoringExternalCred_auth_type.token) {
      if (this.currentObjCredType !==  MonitoringExternalCred_auth_type.token) {
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'bearer-token']))) {
          this.submitButtonTooltip = 'Error: Bearer Token is empty.';
          return false;
        }
      }
    }
    if (this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
          === MonitoringExternalCred_auth_type.certs) {
      if (this.currentObjCredType !==  MonitoringExternalCred_auth_type.certs) {
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'key-data']))) {
          this.submitButtonTooltip = 'Error: Private key is empty.';
          return false;
        }
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'cert-data']))) {
          this.submitButtonTooltip = 'Error: Cert file is empty.';
          return false;
        }
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'ca-data']))) {
          this.submitButtonTooltip = 'Error: CA bundle file is empty.';
          return false;
        }
      } else {
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'key-data'])) &&
            (!this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'cert-data'])) ||
            !this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'ca-data'])))) {
          this.submitButtonTooltip = 'Error: Private key is empty.';
          return false;
        }
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'cert-data'])) &&
            (!this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'key-data'])) ||
            !this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'ca-data'])))) {
          this.submitButtonTooltip = 'Error: Cert file is empty.';
          return false;
        }
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'cert-data'])) &&
            (!this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'cert-data'])) ||
            !this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'key-data'])))) {
          this.submitButtonTooltip = 'Error: CA bundle file is empty.';
          return false;
        }
      }
    }

    this.submitButtonTooltip = 'Ready to submit.';
    return true;
  }

  isNewVcenterIntegrationNameValid(existingObjects: IOrchestrationOrchestrator[]): ValidatorFn {
    // checks if name field is valid
    return Utility.isModelNameUniqueValidator(existingObjects, 'newVcenterIntegration-name');
  }

  getClassName(): string {
    return this.constructor.name;
  }

  setToolbar(): void {
    if (!this.isInline && this.uiconfigsService.isAuthorized(UIRolePermissions.orchestrationorchestrator_create)) {
      // If it is not inline, we change the toolbar buttons, and save the old one
      // so that we can set it back when we are done
      const currToolbar = this._controllerService.getToolbarData();
      this.oldButtons = currToolbar.buttons;
      currToolbar.buttons = [
        {
          cssClass: 'global-button-primary newVcenter-button newVcenter-save',
          text: 'CREATE VCENTER',
          genTooltip: () => this.getSubmitButtonToolTip(),
          callback: () => { this.saveObject(); },
          computeClass: () => this.computeFormSubmitButtonClass()
        },
        {
          cssClass: 'global-button-neutral newVcenter-button newVcenter-cancel',
          text: 'CANCEL',
          callback: () => {
            this.cancelObject();
          }
        },
      ];

      this._controllerService.setToolbarData(currToolbar);
    }
  }

  /**
   * override super api
   * We make up the JSON object
   */
  createObject(newObject: IOrchestrationOrchestrator): Observable<{ body: IOrchestrationOrchestrator | IApiStatus | Error; statusCode: number; }> {
    return this.orchestrationService.AddOrchestrator(newObject);
  }

  /**
   * override super api
   * We make up the JSON object
   */
  updateObject(newObject: IOrchestrationOrchestrator, oldObject: IOrchestrationOrchestrator): Observable<{ body: IOrchestrationOrchestrator | IApiStatus | Error; statusCode: number; }> {
    return this.orchestrationService.UpdateOrchestrator(oldObject.meta.name, newObject, null, oldObject);
  }

  generateCreateSuccessMsg(object: IOrchestrationOrchestrator): string {
    return 'Created vCenter ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: IOrchestrationOrchestrator): string {
    return 'Updated vCenter ' + object.meta.name;
  }

  getObjectValues(): IOrchestrationOrchestrator {
    const currValue: IOrchestrationOrchestrator =  this.newObject.getFormGroupValues();
    if (currValue.spec.credentials['disable-server-authentication'] == null) {
      currValue.spec.credentials['disable-server-authentication'] = true;
    }
    if (this.pickedOption === this.ALL_DATACENTERS) {
      currValue.spec['manage-namespaces'] = [this.ALL_DATACENTERS];
    }
    Utility.removeObjectProperties(currValue, 'status');
    const credential: IMonitoringExternalCred = currValue.spec.credentials;
    if (credential['auth-type'] === MonitoringExternalCred_auth_type['username-password']) {
      Utility.removeObjectProperties(credential, ['bearer-token', 'key-data', 'ca-data', 'cert-data']);
      if (this.isInline && !credential.username) {
        Utility.removeObjectProperties(currValue.spec, 'credentials');
      }
    } else if (credential['auth-type'] === MonitoringExternalCred_auth_type.token) {
      Utility.removeObjectProperties(credential, ['username', 'password', 'key-data', 'ca-data', 'cert-data']);
      if (this.isInline && !credential['bearer-token']) {
        Utility.removeObjectProperties(currValue.spec, 'credentials');
      }
    } else if (credential['auth-type'] === MonitoringExternalCred_auth_type.certs) {
      Utility.removeObjectProperties(credential, ['bearer-token', 'username', 'password']);
      if (this.isInline && !credential['key-data']) {
        Utility.removeObjectProperties(currValue.spec, 'credentials');
      }
    }
    return currValue;
  }

  isNameRequired() {
    return this.isFieldEmpty(this.newObject.$formGroup.get(['meta', 'name']));
  }

  isUriRequired() {
    return this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'uri']));
  }

  isUsernameRequired() {
    if (!this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'username']))) {
      return false;
    }
    if (!this.isInline) {
      return this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
        === MonitoringExternalCred_auth_type['username-password'];
    }
    if (this.currentObjCredType !== MonitoringExternalCred_auth_type['username-password']) {
      return true;
    }
    return !this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'password']));
  }

  isPasswordRequired() {
    if (!this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'password']))) {
      return false;
    }
    if (!this.isInline) {
      return this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
        === MonitoringExternalCred_auth_type['username-password'];
    }
    if (this.currentObjCredType !== MonitoringExternalCred_auth_type['username-password']) {
      return true;
    }
    return false;
  }

  isTokenRequired() {
    if (!this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'bearer-token']))) {
      return false;
    }
    if (!this.isInline) {
      return this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
        === MonitoringExternalCred_auth_type.token;
    }
    return this.currentObjCredType !== MonitoringExternalCred_auth_type.token;
  }

  iskeyRequired() {
    if (!this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'key-data']))) {
      return false;
    }
    if (!this.isInline) {
      return this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
        === MonitoringExternalCred_auth_type.certs;
    }
    if (this.currentObjCredType !== MonitoringExternalCred_auth_type.certs) {
      return true;
    }
    return !this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'cert-data'])) ||
        !this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'ca-data']));
  }

  isCertRequired() {
    if (!this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'cert-data']))) {
      return false;
    }
    if (!this.isInline) {
      return this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
        === MonitoringExternalCred_auth_type.certs;
    }
    if (this.currentObjCredType !== MonitoringExternalCred_auth_type.certs) {
      return true;
    }
    return !this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'key-data'])) ||
        !this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'ca-data']));
  }

  isCaRequired() {
    if (!this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'ca-data']))) {
      return false;
    }
    if (!this.isInline) {
      return this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
        === MonitoringExternalCred_auth_type.certs;
    }
    if (this.currentObjCredType !== MonitoringExternalCred_auth_type.certs) {
      return true;
    }
    return !this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'cert-data'])) ||
        !this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'key-data']));
  }

  isValidDCName(name: string): boolean {
    // put a place holder here for the futre validation
    return true;
  }
}
