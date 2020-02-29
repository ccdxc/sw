
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
export class NewdscprofileComponent extends CreationForm<IClusterDSCProfile, ClusterDSCProfile> implements OnInit, AfterViewInit , OnDestroy {
  @Input() existingObjects: ClusterDSCProfile[] = [];

  fwdModeOptions: SelectItem[] = Utility.convertEnumToSelectItem(ClusterDSCProfileSpec.propInfo['fwd-mode'].enum);
  policyModeOptions: SelectItem[] = Utility.convertEnumToSelectItem(ClusterDSCProfileSpec.propInfo['policy-mode'].enum);

  selectedFwdMode: SelectItem[] = [];
  selectedPolicyMode: SelectItem[] = [];

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

  setCustomValidation () {
    // TODO: remove this if-block once dscprofile.proto is fixed.
    if (!this.newObject.$formGroup) {
      return;
    }
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
          computeClass: () => this.computeButtonClass()
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
  createObject(object: IClusterDSCProfile) {
    return this.clusterService.AddDSCProfile(object, '', true, false);
  }
  updateObject(newObject: IClusterDSCProfile, oldObject: IClusterDSCProfile) {
    return this.clusterService.UpdateDSCProfile(oldObject.meta.name, newObject, null, oldObject, true, false);
  }

  isFormValid(): boolean {
    return true;
  }

  isFieldEmpty(field: AbstractControl): boolean {
    return Utility.isEmpty(field.value);
  }
}
