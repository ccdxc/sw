import { Component, OnInit, ViewEncapsulation, ChangeDetectorRef, Output, EventEmitter, Input, OnChanges, SimpleChanges, AfterContentInit } from '@angular/core';
import { required } from '@sdk/v1/utils/validators';
import { AuthpolicybaseComponent } from '@app/components/admin/authpolicy/authpolicybase/authpolicybase.component';
import { Animations } from '@app/animations';
import {IAuthRadius, AuthRadius, AuthRadiusServer, AuthRadiusDomain} from '@sdk/v1/models/generated/auth';
import { RadiusSave } from '@app/components/admin/authpolicy/.';
import { FormArray, AbstractControl } from '@angular/forms';
import { SelectItem } from 'primeng/primeng';
import { Utility } from '@app/common/Utility';
import { ControllerService } from '@app/services/controller.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';

/**
 * RadiusComponent is a child component of AuthPolicy.component (parent)
 * parent passes in "radiusData" to child, child has a "radiusObject" to reflect "RadiusData".
 *
 * When user wants to save Radius, RadiusComponent emits event to parent and let parent handles the REST calls.
 *
 * Validation:
 * 2019-01-24, if we change auth.proto  validation settings, it will cause a lot of go tests failures.
 * We set validation in UI instead.  see setRadiusValidationRules() API
 */
@Component({
  selector: 'app-radius',
  templateUrl: './radius.component.html',
  styleUrls: ['./radius.component.scss'],
  encapsulation: ViewEncapsulation.None,
  host: {
    '(mouseenter)': 'onMouseEnter()',
    '(mouseleave)': 'onMouseLeave()'
  },
  animations: [Animations]
})
export class RadiusComponent extends AuthpolicybaseComponent implements OnInit, OnChanges, AfterContentInit {
  isHover: boolean = false;
  radiusEditMode: boolean = false;
  inCreateMode: boolean = false;
  radiusObject: AuthRadius = new AuthRadius();

  serverAuthMethodOptions: SelectItem[] = Utility.convertEnumToSelectItem(AuthRadiusServer.propInfo['auth-method'].enum, ['peap-mschapv2', 'eap-ttls-pap']);


  @Input() radiusData: AuthRadius;
  @Output() invokeSaveRadius: EventEmitter<RadiusSave> = new EventEmitter();
  @Output() invokeCreateRadius: EventEmitter<RadiusSave> = new EventEmitter();
  @Output() invokeRemoveRadius: EventEmitter<AuthRadius> = new EventEmitter();

  constructor(protected _controllerService: ControllerService, private cd: ChangeDetectorRef,
    protected uiconfigsService: UIConfigsService) {
    super(_controllerService, uiconfigsService);
  }

  ngOnInit() {
    super.ngOnInit();
    this.radiusObject.domains = [new AuthRadiusDomain()];
    this.radiusObject.setModelToBeFormGroupValues();
  }

  ngAfterContentInit() {
    this.updateRadiusObject();
    this.setRadiusEditMode(false); // set radiusEditMode to false, UI will disable  Radius-enabled slider widget
  }

  ngOnChanges(changes: SimpleChanges) {
    this.updateRadiusObject();
  }

  updateRadiusObject() {
    this.radiusObject.setValues(this.radiusData);
    if (this.radiusObject.domains == null || this.radiusObject.domains.length === 0) {
      const newDomain = new AuthRadiusDomain();
      this.radiusObject.domains = [ newDomain ];
      const domains = this.radiusObject.$formGroup.get(['domains']) as FormArray;
      domains.insert(0, newDomain.$formGroup);
    }
    this.setRadiusValidationRules();
  }

  /**
   * This function is responsible for setting Required Validation on form fields
   */
  private setRadiusValidationRules() {
    this.radiusObject.$formGroup.get(['domains', '0', 'nas-id']).setValidators(required);
    const servers: FormArray = this.radiusObject.$formGroup.get(['domains', '0', 'servers']) as FormArray;
    if (servers.length > 0) {
      const controls = servers.controls;
      for (let i = 0; i < controls.length; i++) {
        const control: AbstractControl = controls[i];
        this.setValidatorOnServerControl(control);
      }
    }
  }

  private setValidatorOnServerControl(control: AbstractControl) {
    control.get('url').setValidators(required);
    control.get('secret').setValidators(required);
    control.get('auth-method').setValidators(required);
  }



  updateRadiusData() {
    if (this.radiusObject) {
      this.radiusData.setValues(this.radiusObject.getFormGroupValues());
    }
  }

  toggleEdit() {
    this.setRadiusEditMode(!this.radiusEditMode);
    if (this.radiusEditMode) {
      // Add a blank server if there is none
      if (this.radiusObject.domains[0].servers.length === 0) {
        this.addServer();
      }
      this.setRadiusValidationRules();
    }
  }

  /**
   * Responsible for deleting selected Radius Policy
   */
  onDeleteRadius() {
    this._controllerService.invokeConfirm({
      header: Utility.generateDeleteConfirmMsg('Config', 'RADIUS'),
      message: 'This action cannot be reversed',
      acceptLabel: 'Delete',
      accept: () => {
        this.invokeRemoveRadius.emit(this.radiusData);
      }
    });
  }

  isAllInputsValid(authRadius: AuthRadius): boolean {
    return (!Utility.getAllFormgroupErrors(authRadius.$formGroup));
  }

  /**
   * This api serves html template. It controlls whether to enble "SAVE" button
   */
  enableSaveButton(): boolean {
    return this.isAllInputsValid(this.radiusObject);
  }

  setRadiusEditMode(isInEditMode) {
    this.radiusEditMode = isInEditMode;
  }


  addServer() {
    const servers = this.radiusObject.$formGroup.get(['domains', '0', 'servers']) as FormArray;
    const newServer = new AuthRadiusServer().$formGroup;
    this.setValidatorOnServerControl(newServer);
    servers.insert(0, newServer);
    this.setValidatorOnServerControl(newServer);
    this.cd.detectChanges(); // this is the trick to avoid ExpressionChangedAfterItHasBeenCheckedError
  }

  removeServer(index) {
    const servers = this.radiusObject.$formGroup.get(['domains', '0', 'servers']) as FormArray;
    if (servers.length > 1) {
      servers.removeAt(index);
    }
  }

  /**
   * Using this function, user returns back to view mode.
   * If this is called during creation, then all data is deleted i.e. form group gets reset.
   * Else the previous data that was there before this editing is preserved
   */
  cancelEdit() {
    this.setRadiusEditMode(false);
    if (this.inCreateMode) {
      // create from is canceling,
      this.radiusData = null;
      this.radiusObject.$formGroup.reset();
    }
    // Reset the radiusObject with the passed in data
    this.updateRadiusObject();
    this.inCreateMode = false;
  }

  /**
   * This function can be only called when all form validation rules are met
   * during creation or updation. Emits Radius data to the parent component,
   * responsible for REST calls, to save the object.
   */
  saveRadius() {
    if (!this.isAllInputsValid(this.radiusObject)) {
      this._controllerService.invokeErrorToaster('Invalid', 'There are invalid inputs.  Fields with "*" are required');
      return;
    }
    this.updateRadiusData();
    let radiusSave: RadiusSave;
    if (this.inCreateMode) {
      radiusSave =  { createData: this.radiusData,
        onSuccess: (resp) => {this.setRadiusEditMode(false); },
      };
      this.invokeCreateRadius.emit(radiusSave);
    } else {
      // POST DATA
      radiusSave =  {
        onSuccess: (resp) => {this.setRadiusEditMode(false); },
      };
      this.invokeSaveRadius.emit(radiusSave); // emit event to parent to update RADIUS if REST call succeeds, ngOnChange() will bb invoked and refresh data.
    }
  }

  /**
   * This function sets new form for creation of the Radius policy
   */
  createRadius() {
    this.radiusData = new AuthRadius();
    this.toggleEdit();
    this.inCreateMode = true;
    this.setRadiusEditMode(true);
  }

}
