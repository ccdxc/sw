
import { AfterViewInit, Component, Input, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { ValidatorFn } from '@angular/forms';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterDSCProfile, ClusterDSCProfileSpec, IClusterDSCProfile, ClusterDSCProfileSpec_deployment_target, ClusterDSCProfileSpec_feature_set } from '@sdk/v1/models/generated/cluster';
import { SelectItem } from 'primeng/primeng';
import { PropInfoItem } from '@sdk/v1/models/generated/basemodel/base-model';
import { DSCProfileUtil, DSCProfileUIModel } from '../dscprofileUtil';



@Component({
  selector: 'app-newdscprofile',
  templateUrl: './newdscprofile.component.html',
  styleUrls: ['./newdscprofile.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewdscprofileComponent extends CreationForm<IClusterDSCProfile, ClusterDSCProfile> implements OnInit, AfterViewInit, OnDestroy {
  @Input() existingObjects: ClusterDSCProfile[] = [];

  selectedFwdMode: SelectItem;
  selectedPolicyMode: SelectItem;
  validationErrorMessage: string;

  constructor(protected _controllerService: ControllerService,
    private clusterService: ClusterService,
    protected uiconfigsService: UIConfigsService
  ) {
    super(_controllerService, uiconfigsService, ClusterDSCProfile);
  }

  depolymentTargetOptions: SelectItem[] = Utility.convertEnumToSelectItem(ClusterDSCProfileSpec_deployment_target);

  featureSets: SelectItem[] = Utility.convertEnumToSelectItem(ClusterDSCProfileSpec_feature_set);

  hostFeatureSets: SelectItem[] = this.featureSets.slice(0, 2);

  virtualizedFeatureSets: SelectItem[] = this.featureSets.slice(2);

  // NEW 5/8/2020
  selectedDeploymentTarget: SelectItem;
  selectedFeatureSet: SelectItem;

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
    if (this.isInline) {
      const dscProfile: ClusterDSCProfile = this.newObject.getFormGroupValues();
      if (dscProfile.spec['deployment-target'] === DSCProfileUtil.DTARGET_HOST) {
        this.selectedDeploymentTarget = this.depolymentTargetOptions[0];
        if (dscProfile.spec['feature-set'] === DSCProfileUtil.FSET_SMARTNIC) {
          this.selectedFeatureSet = this.hostFeatureSets[0];
        } else {
          this.selectedFeatureSet = this.hostFeatureSets[1];
        }
      } else {
        this.selectedDeploymentTarget = this.depolymentTargetOptions[1];
        this.selectedFeatureSet = this.virtualizedFeatureSets[0];
      }
    }
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

  getFeatureSetOptions(): SelectItem[] {
    if (this.selectedDeploymentTarget && this.selectedDeploymentTarget.value === 'host') {
      return this.hostFeatureSets;
    } else if (this.selectedDeploymentTarget && this.selectedDeploymentTarget.value === 'virtualized') {
      return this.virtualizedFeatureSets;
    }
    return [];
  }

  resetFeatureSet() {
    if (this.selectedFeatureSet) {
      this.selectedFeatureSet = null;
    }
  }

  /**
   * Override parent API
   * We use UI-model to update backend-model
   *
   * not form validation; setting value equivalents of user selection in new object
   */
  getObjectValues(): IClusterDSCProfile {
    const dscProfile: ClusterDSCProfile = this.newObject.getFormGroupValues();
    dscProfile.spec['deployment-target'] = this.selectedDeploymentTarget.value;
    dscProfile.spec['feature-set'] = this.selectedFeatureSet.value;
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
    if (!this.selectedDeploymentTarget) {
      this.validationErrorMessage = 'Error: Please select a Deployment Target';
      return false;
    }
    if (!this.selectedFeatureSet) {
      this.validationErrorMessage = 'Error: Please select a Feature Set';
      return false;
    }

    return true;
  }

   /**
    * Override super API
    * In case update fails, restore original value;
    * @param isCreate
    */
   onSaveFailure(isCreate: boolean) {
     if (! isCreate) { // it is an update operation failure
      this._controllerService.invokeInfoToaster('Info', 'Restore original value to DSC Profile.');
      this.newObject.setFormGroupValuesToBeModelValues();
     }
   }
}
