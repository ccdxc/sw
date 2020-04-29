
import { AfterViewInit, Component, Input, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { ValidatorFn } from '@angular/forms';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterDSCProfile, ClusterDSCProfileSpec, IClusterDSCProfile, ClusterFeatureSet } from '@sdk/v1/models/generated/cluster';
import { SelectItem } from 'primeng/primeng';
import { PropInfoItem } from '@sdk/v1/models/generated/basemodel/base-model';


@Component({
  selector: 'app-newdscprofile',
  templateUrl: './newdscprofile.component.html',
  styleUrls: ['./newdscprofile.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewdscprofileComponent extends CreationForm<IClusterDSCProfile, ClusterDSCProfile> implements OnInit, AfterViewInit, OnDestroy {
  @Input() existingObjects: ClusterDSCProfile[] = [];

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
    if (!this.isInline) {
      // When creating a new DSC profile, pre-populate all feature-set keys to true. Thus, user has to do the least amount of work.
      const keys = this.getObjectKeys(this.newObject.spec['feature-set']);
      keys.forEach(key => this.newObject.$formGroup.get(['spec', 'feature-set', key]).setValue(true) );
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



  /**
   * Override parent API
   * We use UI-model to update backend-model
   */
  getObjectValues(): IClusterDSCProfile {
    const dscProfile: ClusterDSCProfile = this.newObject.getFormGroupValues();
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
