
import { required } from '@sdk/v1/utils/validators';
import { AfterViewInit, Component, Input, OnInit, ViewEncapsulation, OnDestroy } from '@angular/core';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterDSCProfile, ClusterDSCProfileSpec, IClusterDSCProfile } from '@sdk/v1/models/generated/cluster';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { SelectItem } from 'primeng/primeng';
import { AbstractControl, Validators, ValidatorFn } from '@angular/forms';


@Component({
  selector: 'app-newdscprofile',
  templateUrl: './newdscprofile.component.html',
  styleUrls: ['./newdscprofile.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewdscprofileComponent extends CreationForm<IClusterDSCProfile, ClusterDSCProfile> implements OnInit, AfterViewInit, OnDestroy {
  @Input() existingObjects: ClusterDSCProfile[] = [];

  // start releaseB settings
  // define properties for handling releaes-b use case. DSC-PROFILE only support
  /**
   FW                Policy
  ------------------------------
  insert             Enforce
  Transpart          BaseNet
  Transpart          FlowAware

  html and ts will branch of based on this.isReleaseB value
   */
  isReleaseB: boolean = true;
  releaseBDSCProfileOptions: SelectItem[] = [
    { label: 'FW:Insert POLICY:Enforce', value: { fw: 'insertion', policy: 'enforced' } },
    { label: 'FW:Transparent POLICY:BaseNet', value: { fw: 'transparent', policy: 'basenet' } },
    { label: 'FW:Transparent POLICY:FlowAware', value: { fw: 'transparent', policy: 'flowaware' } }
  ];
  selectedReleaseBDSCProfile: SelectItem;
  // end releaseB settings

  fwdModeOptions: SelectItem[] = Utility.convertEnumToSelectItem(ClusterDSCProfileSpec.propInfo['fwd-mode'].enum);
  policyModeOptions: SelectItem[] = Utility.convertEnumToSelectItem(ClusterDSCProfileSpec.propInfo['policy-mode'].enum);

  selectedFwdMode: SelectItem;
  selectedPolicyMode: SelectItem;
  validationErrorMessage: string;

  constructor(protected _controllerService: ControllerService,
    private clusterService: ClusterService,
    protected uiconfigsService: UIConfigsService
  ) {
    super(_controllerService, uiconfigsService, ClusterDSCProfile);
  }


  generateCreateSuccessMsg(object: IClusterDSCProfile) {
    return 'Created DSC Profile ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: IClusterDSCProfile) {
    return 'Updated DSC Profile ' + object.meta.name;
  }

  getClassName(): string {
    return this.constructor.name;
  }
  postNgInit(): void {
    // do nothing
  }

  setCustomValidation() {
    this.newObject.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
      this.isDSCProfileNameValid(this.existingObjects)]);
  }

  isDSCProfileNameValid(existingTechSupportRequest: ClusterDSCProfile[]): ValidatorFn {
    return Utility.isModelNameUniqueValidator(existingTechSupportRequest, 'DSC-profile-name');
  }

  setToolbar() {
    if (!this.isInline) {
      // If it is not inline, we change the toolbar buttons, and save the old one
      // so that we can set it back when we are done
      const currToolbar = this._controllerService.getToolbarData();
      currToolbar.buttons = [
        {
          cssClass: 'global-button-primary global-button-padding',
          text: 'CREATE DSC PROFILE ',
          callback: () => { this.saveObject(); },
          computeClass: () => this.computeButtonClass(),
          genTooltip: () => this.getTooltip()
        },
        {
          cssClass: 'global-button-primary global-button-padding',
          text: 'CANCEL',
          callback: () => { this.cancelObject(); }
        },
      ];

      this._controllerService.setToolbarData(currToolbar);
    }
  }



  /**
   * Override parent API
   * We use UI-model to update backend-model
   */
  getObjectValues(): IClusterDSCProfile {
    const dscProfile: ClusterDSCProfile = this.newObject.getFormGroupValues();
    if (this.isReleaseB) {
      dscProfile.spec['fwd-mode'] = this.selectedReleaseBDSCProfile.value.fw;
      dscProfile.spec['policy-mode'] = this.selectedReleaseBDSCProfile.value.policy;
    } else {
      dscProfile.spec['fwd-mode'] = this.selectedFwdMode.value;
      dscProfile.spec['policy-mode'] = this.selectedPolicyMode.value;
    }
    return dscProfile;
  }

  createObject(object: IClusterDSCProfile) {
    return this.clusterService.AddDSCProfile(object, '', true, false);
  }
  updateObject(newObject: IClusterDSCProfile, oldObject: IClusterDSCProfile) {
    return this.clusterService.UpdateDSCProfile(oldObject.meta.name, newObject, null, oldObject, true, false);
  }

  getTooltip(): string {
    return (this.isFormValid()) ? 'Save DSC Profile' : this.validationErrorMessage;
  }

  isFormValid(): boolean {
    this.validationErrorMessage = null;
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      this.validationErrorMessage = 'Error: Name field is empty.';
      return false;
    }
    if (!this.newObject.$formGroup.valid) {
      this.validationErrorMessage = 'Error: Name field is not valid.';
      return false;
    }
    if (this.isReleaseB && !this.selectedReleaseBDSCProfile) {
      this.validationErrorMessage = 'Please select DSC profile config.';
      return false;
    } if (!this.isReleaseB && (!this.selectedFwdMode || !this.selectedPolicyMode)) {
      this.validationErrorMessage = 'Please select both FW and Policy values.';
      return false;
    }
    return true;
  }

  isFieldEmpty(field: AbstractControl): boolean {
    return Utility.isEmpty(field.value);
  }
}
