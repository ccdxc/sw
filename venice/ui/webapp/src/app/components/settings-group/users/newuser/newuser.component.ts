import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, OnDestroy, OnChanges, ViewEncapsulation } from '@angular/core';
import { Validators, ValidationErrors } from '@angular/forms';
import { Observable, forkJoin } from 'rxjs';

import { UsersComponent, ACTIONTYPE } from '../users.component';
import { ErrorStateMatcher } from '@angular/material';
import { Animations } from '@app/animations';
import { SelectItem } from 'primeng/primeng';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/generated/auth.service';
import { StagingService } from '@app/services/generated/staging.service';
import { AuthRoleBinding, AuthUser } from '@sdk/v1/models/generated/auth';
import { Utility } from '@app/common/Utility';
import { StagingBuffer } from '@sdk/v1/models/generated/staging';
import { AbstractControl, ValidatorFn } from '@angular/forms';

import { required } from '@sdk/v1/models/generated/auth/validators.ts';


@Component({
  selector: 'app-newuser',
  templateUrl: './newuser.component.html',
  styleUrls: ['./newuser.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewuserComponent extends UsersComponent implements OnInit, AfterViewInit, OnChanges, OnDestroy {

  newAuthUser: AuthUser;
  selectedRolebindingsForUsers: AuthRoleBinding[] = [];

  @Input() authRoleBindings: AuthRoleBinding[] = [];
  @Input() authUsers: AuthUser[] = [];
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  @Input() rolebindingOptions: SelectItem[] = [];

  errorChecker = new ErrorStateMatcher();

  protected rolebindingUpdateMap = {};

  constructor(protected _controllerService: ControllerService,
    protected _authService: AuthService,
    protected stagingService: StagingService
  ) {
    super(_controllerService, _authService, stagingService);
  }

  ngOnInit() {
    this.newAuthUser = new AuthUser();
    // set validation rules.  Angular does not have api to get existing validators. Thus, we reset all validators
    // meta.nam is required and must be unique
    this.newAuthUser.$formGroup.get(['meta', 'name']).setValidators([required, this.isUsernameValid(this.authUsers)]);
    // spec.password is required.
    this.newAuthUser.$formGroup.get(['spec', 'password']).setValidators([required]);
  }

  ngAfterViewInit() {

  }

  /**
   * This is a customized validator function.
   * We don't want new user name not duplicated with existing users.
   *
   * @param authUsers
   */
  isUsernameValid(authUsers: AuthUser[]): ValidatorFn {
    return (control: AbstractControl): ValidationErrors| null => {
      if (this.isUserAlreadyExist(control.value, authUsers)) {
        return { 'login-name': {
            required: true,
            message: 'Login name is required and must be unique'
        } };
      }
      return null;
    };
  }

  isErrorState(control) {
    return this.errorChecker.isErrorState(control, null);
  }

  /**
   * override super.api()
   */
  ngOnDestroy() { }

  ngOnChanges() {
    this.populateRolebindingOptions();
    this.selectedRolebindingsForUsers.length = 0;
  }

  getClassName(): string {
    return this.constructor.name;
  }

  addUser() {
    const newUser = this.newAuthUser.getFormGroupValues();
    if (this.isUserAlreadyExist(newUser.meta.name, this.authUsers)) {
      this.messageService.add({
        severity: 'error',
        summary: 'Can not create user',
        detail: newUser.meta.name + ' already exist'
      });
      return;
    }
    this.addUser_with_staging();
  }

  /**
   * create a user - UI will let admin to bind user to roles.  (Say  user “joe blow” is created, admin wants to bind “joeblow" operator role and support role)
   *        create buffer
   *        if (success)
   *           add user
   *           add “joeblow” to  operator_rolebinding.user[] list
   *           add “joeblow” to  support_rolebinding.user[] list
   *          if (success)
   *             commit buffer
   *           else
   *             delete buffer
   */
 addUser_with_staging() {
    const newUser = this.newAuthUser.getFormGroupValues();
    this.createStagingBuffer().subscribe(
      responseBuffer => {
        const createdBuffer: StagingBuffer = responseBuffer.body as StagingBuffer;
        const buffername = createdBuffer.meta.name;
        const observables: Observable<any>[] = [];
        const username = newUser.meta.name;
        observables.push(this._authService.AddUser(newUser, buffername));
        this.selectedRolebindingsForUsers.forEach((rolebinding) => {
          this.rolebindingUpdateMap[rolebinding.meta.name] = false;
          const observabe = this.addUserToRolebindings(rolebinding, username, buffername);
          if (observabe) {
            observables.push(observabe);
          }
        });
        forkJoin(observables).subscribe(results => {
          const isAllOK = this.isForkjoinResultAllOK(results);
          if (isAllOK) {
            this.commitStagingBuffer(buffername).subscribe(
              responseCommitBuffer => {
                this.invokeSuccessToaster('Successful', ACTIONTYPE.CREATE + ' User ' + newUser.meta.name);
                this.formClose.emit(true);
              },
              error => {
                console.error('Fail to commit Buffer', error);
                this.invokeRESTErrorToaster('Fail to commit buffer when adding user ' ,  error);
              }
            );
          } else {
            this.deleteStagingBuffer(buffername, 'Fail to add user');
          }
        });
      },
      this.restErrorHandler('Fail to create commit buffer when adding user')
    );
  }




  /**
   * Override super.api(..)
   *
   */
  setDataReadyMap(key: string, value: boolean = true) {
    this.rolebindingUpdateMap[key] = value;
    const self = this;
    const keys = Object.keys(this.rolebindingUpdateMap);
    let allset = true;
    keys.filter((item) => {
      if (self.rolebindingUpdateMap[item] === false) {
        allset = false;
      }
    });
    if (allset) {
      this.formClose.emit(true);
    }
  }


  onSaveAddUser($event) {
    const errors = Utility.getAllFormgroupErrors(this.newAuthUser.$formGroup);
    if (errors === null) {
      this.addUser();
    }
  }

  onCancelAddUser($event) {
    this.newAuthUser.$formGroup.reset();
    this.selectedRolebindingsForUsers.length = 0;
    this.formClose.emit(false);
  }

  onRolebindingChange($event) {
    this.selectedRolebindingsForUsers = $event.value;
  }

  isAllInputsValidated() {
    const hasFormGroupError = Utility.getAllFormgroupErrors(this.newAuthUser.$formGroup);
    const hasSelectedRole = (this.selectedRolebindingsForUsers && this.selectedRolebindingsForUsers.length > 0);
    return (hasFormGroupError === null) && (hasSelectedRole);
  }

}
