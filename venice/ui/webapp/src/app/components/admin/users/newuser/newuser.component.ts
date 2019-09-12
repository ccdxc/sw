import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, OnDestroy, OnChanges, ViewEncapsulation } from '@angular/core';
import { ValidationErrors } from '@angular/forms';
import { Observable, forkJoin } from 'rxjs';
import { switchMap } from 'rxjs/operators';

import { UsersComponent, ACTIONTYPE } from '../users.component';
import { Animations } from '@app/animations';
import { SelectItem } from 'primeng/primeng';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/generated/auth.service';
import { StagingService } from '@app/services/generated/staging.service';
import { AuthRoleBinding, AuthUser } from '@sdk/v1/models/generated/auth';
import { Utility } from '@app/common/Utility';
import { StagingBuffer } from '@sdk/v1/models/generated/staging';
import { AbstractControl, ValidatorFn } from '@angular/forms';

import { required , patternValidator} from '@sdk/v1/utils/validators';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { hasStagingPermission, invokeConfigureStagingPermission } from '../';

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

  rolebindingUpdateMap = {};

  constructor(protected _controllerService: ControllerService,
    protected _authService: AuthService,
    protected stagingService: StagingService,
    protected _uiconfigsService: UIConfigsService
  ) {
    super(_controllerService, _authService, stagingService, _uiconfigsService);
  }

  ngOnInit() {
    this.newAuthUser = new AuthUser();
    // set validation rules.  Angular does not have api to get existing validators. Thus, we reset all validators
    // meta.nam is required and must be unique
    this.newAuthUser.$formGroup.get(['meta', 'name']).setValidators([
      this.newAuthUser.$formGroup.get(['meta', 'name']).validator,
      this.isUsernameValid(this.authUsers)
    ]);
    this.newAuthUser.$formGroup.get(['spec', 'password']).setValidators([required, patternValidator(UsersComponent.PASSWORD_REGEX, UsersComponent.PASSWORD_MESSAGE )]);
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
    return (control: AbstractControl): ValidationErrors | null => {
      if (this.isUserAlreadyExist(control.value, authUsers)) {
        return {
          'login-name': {
            required: true,
            message: 'Login name is required and must be unique'
          }
        };
      }
      return null;
    };
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
      this._controllerService.invokeErrorToaster(Utility.CREATE_FAILED_SUMMARY,
        newUser.meta.name + ' already exist');
      return;
    }
    if (hasStagingPermission(this._uiconfigsService)) {
      this.addUser_with_staging();
    } else {
      invokeConfigureStagingPermission(this._controllerService);
      return;
    }
  }

  /**
   * create a user - UI will let admin to bind user to roles.  (Say  user “joe blow” is created, admin wants to bind “joeblow" operator role and support role)
   *        create buffer
   *        if (success)
   *           add user
   *            if (success)
   *                add “joeblow” to  operator_rolebinding.user[] list
   *                add “joeblow” to  support_rolebinding.user[] list
   *
   *          if (success)
   *             commit buffer
   *           else
   *             delete buffer
   */
  addUser_with_staging() {
    if (!hasStagingPermission(this.uiconfigsService)) {
      invokeConfigureStagingPermission(this._controllerService);
      return;
    }
    const newUser = this.newAuthUser.getFormGroupValues();
    let createdBuffer: StagingBuffer = null;  // responseBuffer.body as StagingBuffer;
    let buffername = null; // createdBuffer.meta.name;
    const observables: Observable<any>[] = [];
    const username = newUser.meta.name;
    this.createStagingBuffer() // invoke REST call (A)
      .pipe(
        switchMap(responseBuffer => {  // get server response from (A))
          createdBuffer = responseBuffer.body as StagingBuffer;
          buffername = createdBuffer.meta.name;
          return this._authService.AddUser(newUser, buffername).pipe(
            switchMap(user => {
              this.selectedRolebindingsForUsers.forEach((rolebinding) => {
                this.rolebindingUpdateMap[rolebinding.meta.name] = false;
                const observabe = this.addUserToRolebindings(rolebinding, username, buffername);
                if (observabe) {
                  observables.push(observabe);
                }
              });
              if (observables.length > 0) {
              // Update all role-binding - add newly created usernam to selected role-binding
              return this.invokeForkJoin(observables, buffername);
              } else {
                return this.commitStagingBuffer(buffername);
              }
            })
          );
        })
      ).subscribe(
        // We are getting response for (B)
        (responseCommitBuffer) => {
          this._controllerService.invokeSuccessToaster(Utility.CREATE_SUCCESS_SUMMARY, ACTIONTYPE.CREATE + ' User ' + newUser.meta.name);
          this.formClose.emit(true);
        },
        (error) => {
          // any error in (A) (B) or (C), error will land here
          this._controllerService.invokeRESTErrorToaster(Utility.CREATE_FAILED_SUMMARY, error);
          this.deleteStagingBuffer(buffername, Utility.CREATE_FAILED_SUMMARY, false);
        }
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
