import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation, OnDestroy, ViewChild, ChangeDetectorRef } from '@angular/core';
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

@Component({
  selector: 'app-newvcenter-integration',
  templateUrl: './newVcenterIntegration.component.html',
  styleUrls: ['./newVcenterIntegration.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewVcenterIntegrationComponent extends CreationForm<IOrchestrationOrchestrator, OrchestrationOrchestrator> implements OnInit, AfterViewInit, OnDestroy {

  @Input() isInline: boolean = false;
  @Input() existingObjects: IOrchestrationOrchestrator[] = [];

  createButtonTooltip: string = 'Ready to submit.';

  credentialTypes = Utility.convertEnumToSelectItem(MonitoringExternalCred_auth_type);
  currentObjCredType: string = 'none';

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected orchestrationService: OrchestrationService
  ) {
    super(_controllerService, uiconfigsService, OrchestrationOrchestrator);
  }

  postNgInit() {
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
    const credForm: FormGroup = newObject.$formGroup.get(['spec', 'credentials']) as FormGroup;
    credForm.addControl('confirmPassword', CustomFormControl(new FormControl('', [
      Utility.isControlValueMatchOtherControlValueValidator(
        newObject.$formGroup.get(['spec', 'credentials', 'password']),
        'confirmPassword',
        Utility.CONFIRM_PASSWORD_MESSAGE)
    ]), {}));
  }

  processVcenterIntegration() {
    this.currentObjCredType =
      this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value;
    // clear crdentials
    if (this.currentObjCredType === MonitoringExternalCred_auth_type['username-password']) {
      this.newObject.$formGroup.get(['spec', 'credentials', 'username']).setValue('');
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

  getTooltip(): string {
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      return 'Error: Name field is empty.';
    }
    if (this.newObject.$formGroup.get(['meta', 'name']).invalid)  {
      return 'Error: Name field is invalid.';
    }
    return this.createButtonTooltip;
  }

  isFormValid(): boolean {
    if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'uri']))) {
      this.createButtonTooltip = 'Error: URI is empty.';
      return false;
    }
    if (!this.isInline) {
      if (this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
          === MonitoringExternalCred_auth_type['username-password']) {
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'username']))) {
          this.createButtonTooltip = 'Error: Username is empty.';
          return false;
        }
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'password']))) {
          this.createButtonTooltip = 'Error: Password is empty.';
          return false;
        }
        if (this.newObject.$formGroup.get(['spec', 'credentials', 'password']).value !==
            this.newObject.$formGroup.get(['spec', 'credentials', 'confirmPassword']).value) {
          this.createButtonTooltip = 'Error: Password does not match.';
          return false;
        }
      }
      if (this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
          === MonitoringExternalCred_auth_type.token) {
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'bearer-token']))) {
          this.createButtonTooltip = 'Error: Bearer Token is empty.';
          return false;
        }
      }
      if (this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
          === MonitoringExternalCred_auth_type.certs) {
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'key-data']))) {
          this.createButtonTooltip = 'Error: Private key is empty.';
          return false;
        }
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'cert-data']))) {
          this.createButtonTooltip = 'Error: Cert file is empty.';
          return false;
        }
        if (this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'ca-data']))) {
          this.createButtonTooltip = 'Error: CA bundle file is empty.';
          return false;
        }
      }
    } else {
      if (this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
          === MonitoringExternalCred_auth_type['username-password']) {
        if (this.newObject.$formGroup.get(['spec', 'credentials', 'password']).value &&
            this.newObject.$formGroup.get(['spec', 'credentials', 'password']).value !==
            this.newObject.$formGroup.get(['spec', 'credentials', 'confirmPassword']).value) {
          this.createButtonTooltip = 'Error: Password does not match.';
          return false;
        }
        if (this.newObject.$formGroup.get(['spec', 'credentials', 'confirmPassword']).value &&
            this.newObject.$formGroup.get(['spec', 'credentials', 'confirmPassword']).value !==
            this.newObject.$formGroup.get(['spec', 'credentials', 'password']).value) {
          this.createButtonTooltip = 'Error: Password does not match.';
          return false;
        }
      }
    }
    // ignore confirmPassword validation error
    if (!this.newObject.$formGroup.valid &&
        this.newObject.$formGroup.get(['spec', 'credentials', 'confirmPassword']).valid) {
      this.createButtonTooltip = 'Error: Form is invalid.';
      return false;
    }

    this.createButtonTooltip = 'Ready to submit.';
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
          text: 'Create vCenter',
          genTooltip: () => this.getTooltip(),
          callback: () => { this.saveObject(); },
          computeClass: () => this.computeButtonClass()
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
    return 'Created vCenter Integration ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: IOrchestrationOrchestrator): string {
    return 'Updated vCenter Integration ' + object.meta.name;
  }

  getObjectValues(): IOrchestrationOrchestrator {
    const currValue: IOrchestrationOrchestrator =  this.newObject.getFormGroupValues();
    if (!this.isInline) {
      currValue.meta.labels = {
        'force-dc-names': Utility.getGUID()
      };
    }
    delete currValue.status;
    const formGrp: any = currValue.spec.credentials;
    delete formGrp.confirmPassword;
    const credential: IMonitoringExternalCred = currValue.spec.credentials;
    if (credential['auth-type'] === MonitoringExternalCred_auth_type['username-password']) {
      delete credential['bearer-token'];
      delete credential['key-data'];
      delete credential['ca-data'];
      delete credential['cert-data'];
      if (!credential.username) {
        delete credential.username;
      }
      if (!credential.password) {
        delete credential.password;
      }
    } else if (credential['auth-type'] === MonitoringExternalCred_auth_type.token) {
      delete credential['key-data'];
      delete credential['ca-data'];
      delete credential['cert-data'];
      delete credential.username;
      delete credential.password;
      if (!credential['bearer-token']) {
        delete credential['bearer-token'];
      }
    } else if (credential['auth-type'] === MonitoringExternalCred_auth_type.certs) {
      delete credential['bearer-token'];
      delete credential.username;
      delete credential.password;
      if (!credential['key-data']) {
        delete credential['key-data'];
      }
      if (!credential['ca-data']) {
        delete credential['ca-data'];
      }
      if (!credential['cert-data']) {
        delete credential['cert-data'];
      }
    }
    return currValue;
  }

  isFieldEmpty(field: AbstractControl): boolean {
    return Utility.isEmpty(field.value);
  }

  isNameRequired() {
    return this.isFieldEmpty(this.newObject.$formGroup.get(['meta', 'name']));
  }

  isUriRequired() {
    return this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'uri']));
  }

  isCredentialRequired(field: string, type: string) {
    return (!this.isInline) &&
      this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
        === MonitoringExternalCred_auth_type[type] &&
      this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', field]));
  }

  isConfirmPwdRequired() {
    return this.newObject.$formGroup.get(['spec', 'credentials', 'auth-type']).value
        === MonitoringExternalCred_auth_type['username-password'] &&
      !this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'password'])) &&
      this.isFieldEmpty(this.newObject.$formGroup.get(['spec', 'credentials', 'confirmPassword']));
  }
}
