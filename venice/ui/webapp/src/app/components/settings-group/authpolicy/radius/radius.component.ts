import { Component, OnInit, ViewEncapsulation, Output, EventEmitter, Input, OnChanges, SimpleChanges, AfterContentInit } from '@angular/core';
import { Validators } from '@angular/forms';
import { AuthpolicybaseComponent } from '@app/components/settings-group/authpolicy/authpolicybase/authpolicybase.component';
import { Animations } from '@app/animations';
import { IAuthRadius, AuthRadius, AuthRadiusServer } from '@sdk/v1/models/generated/auth';
import { FormArray, AbstractControl } from '@angular/forms';
import { SelectItem } from 'primeng/primeng';
import { Utility } from '@app/common/Utility';
import { ControllerService } from '@app/services/controller.service';

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
export class RadiusComponent extends AuthpolicybaseComponent implements OnInit, OnChanges, AfterContentInit  {
  isHover: boolean = false;
  radiusEditMode: boolean = false;
  inCreateMode: boolean = false;
  radiusObject: AuthRadius = new AuthRadius();

  serverAuthMethodOptions: SelectItem[] = Utility.convertEnumToSelectItem(AuthRadiusServer.propInfo['auth-method'].enum);


  @Input() radiusData: AuthRadius;
  @Output() invokeSaveRadius: EventEmitter<Boolean> = new EventEmitter();
  @Output() invokeCreateRadius: EventEmitter<AuthRadius> = new EventEmitter();

  constructor(protected _controllerService: ControllerService) {
    super(_controllerService);
  }

  ngOnInit() {
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
    this.setRadiusValidationRules();
  }

  private setRadiusValidationRules() {
    const servers: FormArray = this.radiusObject.$formGroup.get('servers') as FormArray;
    if (servers.length > 0) {
      const controls = servers.controls;
      for (let i = 0; i < controls.length; i++) {
        const control: AbstractControl = controls[i];
        this.setValidatorOnServerControl(control);
      }
    }
  }

  private setValidatorOnServerControl(control: AbstractControl) {
    control.get('url').setValidators(Validators.required);
    control.get('secret').setValidators(Validators.required);
    control.get('auth-method').setValidators(Validators.required);
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
      if (this.radiusObject.servers.length === 0) {
        this.addServer();
      }
      this.setRadiusValidationRules();
    }
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

  setRadiusEnableControl() {
    if (this.radiusEditMode) {
      this.radiusObject.$formGroup.controls['enabled'].enable();
    } else {
      this.radiusObject.$formGroup.controls['enabled'].disable();
    }
  }

  setRadiusEditMode(isInEditMode) {
    this.radiusEditMode = isInEditMode;
    this.setRadiusEnableControl();
  }

  addServer() {
    const servers = this.radiusObject.$formGroup.get('servers') as FormArray;
    const newServer = new AuthRadiusServer().$formGroup;
    servers.insert(0, newServer);
  }

  removeServer(index) {
    const servers = this.radiusObject.$formGroup.get('servers') as FormArray;
    if (servers.length > 1) {
      servers.removeAt(index);
    }
  }

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

  saveRadius() {
    this.updateRadiusData();
    if (this.inCreateMode) {
      if (this.isAllInputsValid(this.radiusObject)) {
        this.invokeCreateRadius.emit(this.radiusData);
      } else {
        this._controllerService.invokeErrorToaster('Invalid', 'There are invalid inputs.  Fields with "*" are requried');
      }
    } else {
      // POST DATA
      this.invokeSaveRadius.emit(false); // emit event to parent to update RADIUS if REST call succeeds, ngOnChange() will bb invoked and refresh data.
    }
  }

  createRadius() {
    this.radiusData = new AuthRadius();
    this.toggleEdit();
    this.inCreateMode = true;
    this.setRadiusEditMode(true);
    this.radiusObject.$formGroup.controls['enabled'].setValue(true);  // set RADIUS enable when set "create RADIUS"
  }

}
