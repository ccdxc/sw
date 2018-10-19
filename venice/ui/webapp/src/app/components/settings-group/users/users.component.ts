import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';

import { SelectItem, MessageService } from 'primeng/primeng';
import { ErrorStateMatcher } from '@angular/material';

import { BaseComponent } from '@app/components/base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';
import { Utility } from '@app/common/Utility';
import { UserDataReadyMap } from './';

import { AuthService } from '@app/services/generated/auth.service';
import {
  IApiStatus, IAuthRoleBindingList, AuthRoleBindingList,
  AuthRoleBinding, IAuthRoleList, AuthRole, IAuthUserList,
  AuthUserList, AuthUser, AuthRoleList
} from '@sdk/v1/models/generated/auth';


/**
 * User component allow end-user manage users.
 * Adding new-user is in newuser.component.ts.   User is corresponding to AuthUser object. AuthUser.status should contain role information which is readonly.
 *
 * AuthUser, AuthRole, AuthRoleBinding (objects) are inter-related.  If assigning a role to user, AuthRoleBinding must update user-name to AuthRoleBinding.spec.users [..]
 * For example, adding "test" to admin-role,  AdminRole (AuthRoleBinding)..spec.users[ 'test' .. ]
 *
 */

@Component({
  selector: 'app-users',
  templateUrl: './users.component.html',
  styleUrls: ['./users.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class UsersComponent extends BaseComponent implements OnInit, OnDestroy {
  authusers: AuthUser[] = [];
  authRoles: AuthRole[] = [];
  authRoleBindings: AuthRoleBinding[] = [];
  selectedAuthUser: AuthUser = null;
  protected rolebindingOptions: SelectItem[] = [];
  protected selectedRolebindings: any[] = [];
  protected rolebindingUpdateMap = {};

  dataReadyMap: UserDataReadyMap = {
    users: false,
    roles: false,
    rolebindings: false
  };

  isToShowAddUserPanel: boolean = false;


  usericon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/auth/ico-authorizations.svg'
  };

  errorChecker = new ErrorStateMatcher();

  constructor(protected _controllerService: ControllerService,
    protected _authService: AuthService,
    protected messageService: MessageService
  ) {
    super(_controllerService, messageService);
  }

  ngOnInit() {
    this.getData();
    this.setToolbarItems();
  }

  protected setToolbarItems() {
    this._controllerService.setToolbarData({
      buttons: [
        {
          cssClass: 'global-button-primary users-toolbar-button',
          text: 'Refresh',
          callback: () => { this.getData(); },
        }
      ],
      breadcrumb: [{ label: 'Authentications', url: '' }, { label: 'Users', url: '' }]
    });
  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to exist
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'UsersComponent', 'state': Eventtypes.COMPONENT_DESTROY });
  }

  getClassName(): string {
    return this.constructor.name;
  }

  setDataReadyMap(key: string, value: boolean = true) {
    this.dataReadyMap[key] = value;
    const keys = Object.keys(this.dataReadyMap);
    let allset = true;
    keys.forEach((item) => {
      if (this.dataReadyMap[item] === false) {
        allset = false;
      }
    });
    if (allset) {
      this.combineData();
    }
  }

  getData() {
    this.getUsers();
    this.getAuthRoles();
    this.getRolebindings();
  }

  combineData() {
    console.log(this.getClassName() + '.combineData()');
    this.populateRolebindingOptions();
  }

  protected populateRolebindingOptions() {
    if (this.authRoleBindings) {
      this.rolebindingOptions.length = 0;
      this.authRoleBindings.forEach((rolebinding) => {
        const obj: SelectItem = {
          label: rolebinding.meta.name,
          value: rolebinding
        };
        this.rolebindingOptions.push(obj);
      });
    }
  }

  getUsers() {
    this._authService.ListUser().subscribe(
      (data) => {
        const authUserList: AuthUserList = new AuthUserList(<IAuthUserList>data.body);
        if (authUserList.Items.length > 0) {
          this.authusers.length = 0;
          this.authusers = authUserList.Items;
          this.setDataReadyMap('users', true);
        }
      },
      this.restErrorHandler('Failed to get Users')
    );
  }

  getAuthRoles() {
    this._authService.ListRole().subscribe(
      (data) => {
        const status = data.statusCode;
        if (status === 200) {
          const authRoleList: AuthRoleList = new AuthRoleList(<IAuthRoleList>data.body);
          if (authRoleList.Items.length > 0) {
            this.authRoles.length = 0;
            this.authRoles = authRoleList.Items;
            this.setDataReadyMap('roles', true);
          }
        } else {
          console.error(this.getClassName() + '_authService.ListRole()' + data);
        }
      },
      this.restErrorHandler('Failed to get Roles')
    );
  }

  getRolebindings() {
    this._authService.ListRoleBinding().subscribe(
      (data) => {
        const authRoleBindings: AuthRoleBindingList = new AuthRoleBindingList(<IAuthRoleBindingList>data.body);
        if (authRoleBindings.Items.length > 0) {
          this.authRoleBindings.length = 0;
          this.authRoleBindings = authRoleBindings.Items;
          this.setDataReadyMap('rolebindings', true);
        }
      },
      this.restErrorHandler('Failed to get Role Bindings')
    );
  }

  /**
   * The API serves HTML template
   * Find unique role list
   */
  getRoles(): string[] {
    const roles: string[] = [];
    const unique = {};

    for (let i = 0; i < this.authusers.length; i++) {
      const user = this.authusers[i];
      const userRoleValues = this.getUserRoleName(user);
      const v = (userRoleValues && userRoleValues.length > 0) ? userRoleValues[0] : 'default';
      if (typeof (unique[v]) === 'undefined') {
        roles.push(v);
      }
      const count = (unique[v]) ? unique[v] : 0;
      unique[v] = count + 1;
    }
    return roles;
  }

  getUserRoleName(user: AuthUser): any {
    const _ = Utility.getLodash();
    return _.result(user, 'status.roles');
  }

  /**
   * This API serves html template
   * Find all users of a role.
   * @param role
   */
  getRoleUsers(role: string): AuthUser[] {
    const _ = Utility.getLodash();
    const users: AuthUser[] = [];
    for (let i = 0; i < this.authusers.length; i++) {
      const user = this.authusers[i];
      const value = _.result(user, 'status.roles');
      const v = value.length > 0 ? value[0] : 'default';
      if (role === v) {
        users.push(user);
      }
    }
    return users;
  }

  /**
   * This API serves html template
   * @param $event
   * @param role
   */
  onAddUser($event) {
    this.isToShowAddUserPanel = true;
  }

  /**
   * This API serves html template
   * @param $event
   * @param role
   */
  onDeleteUser($event, user: AuthUser) {
    const r = confirm('Please confirm to delete user:' + user.spec.fullname);
    if (r === true) {
      this._authService.DeleteUser(user.meta.name).subscribe(
        (data) => {
          // refresh users list
          this.getUsers();
        },
        this.restErrorHandler('Delete User Failed')
      );
    } else {
      return;
    }
  }

  /**
   * This API serves html template
   * @param $event
   * @param role
   */
  onEditUser($event, user: AuthUser) {
    this.selectedAuthUser = user;
    // TODO: back-end should not require have password input when update user. 2018-09-20 note.
    // this.selectedAuthUser.$formGroup.get(['spec', 'password']).setValidators(Validators.required);
  }

  /**
   * This API serves html template
   */
  showEditButton(user: AuthUser) {
    const _ = Utility.getLodash();
    const value = _.result(user, 'status.roles');
    return (value && value.length > 0) || true;
  }

  /**
   * This API serves html template
   */
  showDeleteButton(user: AuthUser) {
    const _ = Utility.getLodash();
    const value = _.result(user, 'status.roles');
    const loginname = Utility.getInstance().getLoginName();
    return ((value && value.length > 0) || true) && (user.meta.name !== loginname);
  }

  /**
   * This API serves html template.
   * If it is a save-user operation, refresh data
   */
  creationFormClose(isSaveData: boolean) {
    if (isSaveData) {
      this.getData();
    }
    this.isToShowAddUserPanel = false;
  }

  /**
   * This API serves html template.
   * Check if user is in edit-mode
   */
  isUserEditMode(user: AuthUser): boolean {
    return (this.selectedAuthUser === user);
  }

  onCancelEditUser($event, user) {
    this.selectedAuthUser = null;
  }

  onSaveEditUser($event, user) {
    console.log(this.getClassName() + '.onSaveEditUser()', user);
    this.updateUser();
  }

  onRolebindingChange($event) {
    console.log(this.getClassName() + '.onRolebindingChange()');
    this.selectedRolebindings = $event.value;
  }

  updateUser() {
    this._authService.UpdateUser(this.selectedAuthUser.meta.name, this.selectedAuthUser).subscribe(
      response => {
        const updatedAuthUser: AuthUser = response.body as AuthUser;
        this.handleAddUpdateUserRESTCallSuccess(updatedAuthUser);
      },
      error => {
        this.handleRESTCallError(error, 'AuthService.AddUser() returned code: ');
      }
    );

  }

  protected handleAddUpdateUserRESTCallSuccess(authUser: AuthUser) {
    if (this.selectedRolebindings && this.selectedRolebindings.length > 0) {
      this.rolebindingUpdateMap = {};
      const username = authUser.meta.name;
      this.selectedRolebindings.filter((rolebinding) => {
        this.rolebindingUpdateMap[rolebinding.meta.name] = false;
        this.addUserToRolebinding(rolebinding, username);
      });
    }
  }

  handleRESTCallError(error: any, callerMessage: string) {
    // TODO: Error handling
    if (error.body instanceof Error) {
      console.log(callerMessage + error.statusCode + ' data: ' + <Error>error.body);
    } else {
      console.log(callerMessage + error.statusCode + ' data: ' + <IApiStatus>error.body);
    }
  }

  isUserAlreadyInRoleBinding(rolebinding: AuthRoleBinding, username: string): boolean {
    return (rolebinding.getValues().spec.users.indexOf(username) >= 0);
  }

  addUserToRolebinding(rolebinding: AuthRoleBinding, username: string) {
    if (this.isUserAlreadyInRoleBinding(rolebinding, username)) {
      return; // Since username is already in rolebinding.users list, we will do nothing;
    }
    rolebinding.getValues().spec.users.push(username);
    // TODO:
    // We update to the rolebinding.getValues().
    // rolebinding.spec.users.push(username); will not work
    this._authService.UpdateRoleBinding(rolebinding.meta.name, rolebinding).subscribe(
      response => {
        const status = response.statusCode;
        this.setDataReadyMap(rolebinding.meta.name, true);
      },
      error => {
        this.handleRESTCallError(error, 'AuthService.AddUser() returned code: ');
      }
    );
  }

}
