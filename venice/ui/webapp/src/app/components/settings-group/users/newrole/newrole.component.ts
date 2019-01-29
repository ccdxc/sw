import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, OnDestroy, OnChanges, ViewEncapsulation, SimpleChanges } from '@angular/core';
import { FormArray, Validators, FormGroup, AbstractControl, ValidatorFn, ValidationErrors } from '@angular/forms';
import { required } from '@sdk/v1/models/generated/auth/validators.ts';
import { UsersComponent } from '../users.component';
import { ErrorStateMatcher } from '@angular/material';
import { Animations } from '@app/animations';
import { SelectItem } from 'primeng/primeng';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/generated/auth.service';
import { StagingService } from '@app/services/generated/staging.service';
import { AuthPermission, AuthPermission_actions } from '@sdk/v1/models/generated/auth';
import { AuthRoleBinding, AuthUser, AuthRole } from '@sdk/v1/models/generated/auth';
import { Utility } from '@app/common/Utility';
import { StagingBuffer, IStagingBuffer } from '@sdk/v1/models/generated/staging';

/**
 * NewroleComponent is included in UsersComponent (parent).
 * Venice user can request create or edit AuthRole and this NewroleComponent will provide UI.
 *
 *  getRoleFromSelectedRole() converts back-end data to UI data.
 *  getRoleFromUI() converts UI data to REST-API compliant data
 *
 *  newAuthRole is the key object.
 *  isEditMode() controls whether to add or update an AuthRole object.
 *
 *  When creating a new ROLE, we use commit-buffer to create a role-binding.
 *  Say, user create a "r1" role, a role-binding "r1_binding" will be created along the way. Admin can then edit "r1_binding".
 */
@Component({
  selector: 'app-newrole',
  templateUrl: './newrole.component.html',
  styleUrls: ['./newrole.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewroleComponent extends UsersComponent implements OnInit, OnDestroy, OnChanges {

  errorChecker = new ErrorStateMatcher();
  newAuthRole: AuthRole;

  @Input() selectedAuthRole: AuthRole;
  @Input() veniceRoles: AuthRole[] = [];  // hosted UI will pass in available AuthRoles
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  groupOptions: SelectItem[] = Utility.convertCategoriesToSelectItem();

  permissionOptions: SelectItem[] = Utility.convertEnumToSelectItem(AuthPermission_actions);


  constructor(protected _controllerService: ControllerService,
    protected _authService: AuthService,
    protected stagingService: StagingService
  ) {
    super(_controllerService, _authService, stagingService);
  }

  /**
   * override super.api()
   */
  getClassName(): string {
    return this.constructor.name;
  }

  ngOnInit() {
    if (!this.newAuthRole) {
      this.setupData();
    }
  }

  setupData() {
    if (this.isEditMode()) {
      this.newAuthRole = this.getRoleFromSelectedRole();
    } else {
      this.newAuthRole = new AuthRole();
      this.newAuthRole.$formGroup.get(['meta', 'name']).setValidators([required, this.isRolenameValid(this.veniceRoles)]);

      const permissions = this.newAuthRole.$formGroup.get(['spec', 'permissions']) as FormArray;
      if (permissions.length === 0) {
        permissions.insert(0, new AuthPermission().$formGroup);
      }
    }
  }

  isRolenameValid(authRoles: AuthRole[]): ValidatorFn {
    return (control: AbstractControl): ValidationErrors| null => {
      if (this.isRoleAlreadyExist(control.value, authRoles)) {
        return { 'role-name': {
            required: true,
            message: 'Role name is required and must be unique'
        } };
      }
      return null;
    };
  }

  getPermissionActionItem(element: string): any {
    const items = this.permissionOptions.filter(item => {
      return (item.value === element);
    });
    return items[0];
  }

  /**
   * Take selectedAuthRole JSON value and make a new AuthRole object.
   *  We have to convert permissions[i].actions and permissions[i]['resource-names'] to UI friendly data structure
   */
  getRoleFromSelectedRole(): AuthRole {
    const newRole = this.selectedAuthRole.getFormGroupValues();
    const permissions = newRole.spec.permissions;
    for (let i = 0; i < permissions.length; i++) {
      const actions = permissions[i].actions;
      // actions is like ["create" "update"], we want to change to [{label:value, value=create}, {label:update, value=update}]
      if (actions) {
        const actionsValues = [];
        actions.forEach(element => {
          const item = this.getPermissionActionItem(element);
          if (item) {
            actionsValues.push(item);
          }
        });
        permissions[i].actions = actionsValues;
      }
      const resourcenames = permissions[i]['resource-names'];
      // resourcenames is ["ns1", "ns2"]
      if (resourcenames && Array.isArray(resourcenames)) {
        const rnValues = [];
        resourcenames.forEach(item => {
          rnValues.push(item.trim());
        });
        permissions[i]['resource-names'] = rnValues;
      }
    }
    return new AuthRole(newRole);
  }

  ngOnChanges(changes: SimpleChanges) {
    this.setupData();
  }

  isEditMode(): boolean {
    return (this.selectedAuthRole) ? true : false;
  }


  isErrorState(control) {
    return this.errorChecker.isErrorState(control, null);
  }

  /**
   * override super.api()
   */
  ngOnDestroy() { }

  /**
   * Serves HTML template.
   */
  isAllInputsValidated() {
    const hasFormGroupError = Utility.getAllFormgroupErrors(this.newAuthRole.$formGroup);
    return (hasFormGroupError === null);
  }

  /**
   * Serves HTML template.  User clicks [SAVE] button
   * @param $event
   */
  onSaveAddRole($event) {
    const errors = Utility.getAllFormgroupErrors(this.newAuthRole.$formGroup);
    if (errors === null) {
      if (this.isEditMode()) {
        this.updateRole();
      } else {
        this.addRole();
      }
    }
  }

  /**
   * Serves HTML template.  User clicks [CANCEL] button
   * @param $event
   */
  onCancelAddRole($event) {
    this.newAuthRole.$formGroup.reset();
    this.selectedRolebindingsForUsers.length = 0;
    this.formClose.emit(false);
  }

  updateRole() {
    const newRole = this.getRoleFromUI();
    this._authService.UpdateRole(newRole.meta.name, newRole).subscribe(
      response => {
        const createdRole: AuthRole = response.body as AuthRole;

        this.invokeSuccessToaster('Update Successful', 'Ureated Role ' + newRole.meta.name);
        this.formClose.emit(true);
      },
      this.restErrorHandler('Update Role Failed')
    );
  }

  addRole() {
    this.addRole_with_staging();
  }

  /**
   * Use commit-buffer to create role and role-binding.
   * create buffer
	 *			create roleA
	 *			    create a roleA-rolebinding
   *            commit buffer
   */
  addRole_with_staging() {
    const newRole = this.getRoleFromUI();

    this.createStagingBuffer().subscribe(
      responseBuffer => {
        const createdBuffer: StagingBuffer = responseBuffer.body as StagingBuffer;
        const buffername = createdBuffer.meta.name;
        this._authService.AddRole(newRole, buffername).subscribe(
          responseAddRole => {
            const createdRole: AuthRole = responseAddRole.body as AuthRole;
            const newRolebinding = new AuthRoleBinding();
            newRolebinding.meta.name = newRole.meta.name + '_binding';
            newRolebinding.spec.role = createdRole.meta.name;
            newRolebinding.setFormGroupValuesToBeModelValues();
            this._authService.AddRoleBinding(newRolebinding, buffername).subscribe(
              responseRoleBinding => {
                this.commitStagingBuffer(buffername).subscribe(
                  responseCommitBuffer => {
                    this.invokeSuccessToaster('Creation Successful', 'Created Role ' + newRole.meta.name + ' , And Role-binding ' + newRolebinding.meta.name );
                    this.formClose.emit(true);
                  }
                );
              },
              error => {
                this._controllerService.invokeErrorToaster('Error', 'Create Rolebinding Failed (with commit buffer)');
                this.deleteStagingBuffer(buffername, 'Fail to create role-binding');
              }
            );
          },
          error => {
            this._controllerService.invokeErrorToaster('Error', 'Create Role Failed');
            this.deleteStagingBuffer(buffername, 'Fail to create role');
          }
        );
      },
      this.restErrorHandler('Create buffer failed when creating an role ')
    );
  }


  /**
   * Build Role create/update JSON data.
   * UI has special data structure in permission[i].actions and permissions[i]['resource-names']
   * We have to convert them to REST API complient data structure.
   */
  getRoleFromUI(): any {
    const newRole = this.newAuthRole.getFormGroupValues();
    const permissions = newRole.spec.permissions;
    for (let i = 0; i < permissions.length; i++) {
      const actions = permissions[i].actions;
      if (actions) {
        const actionsValues = [];
        actions.forEach(element => {
          if (element.value) {
            actionsValues.push(element.value);
          }
        });
        permissions[i].actions = actionsValues;
      }
      const resourcenames = permissions[i]['resource-names'];
      // resourcenames is "ns1, ns2" //TODO: server-side request to hide resource-names UI.
      if (resourcenames && !Array.isArray(resourcenames)) {
        const rnValues = [];
        const list = resourcenames.split(',');
        list.forEach(item => {
          rnValues.push(item.trim());
        });
        permissions[i]['resource-names'] = rnValues;
      }
    }
    return newRole;
  }

  /**
   * Server HTML template
   */
  addPermission() {
    const permissionArray = this.newAuthRole.$formGroup.get(['spec', 'permissions']) as FormArray;
    permissionArray.insert(0, new AuthPermission().$formGroup);
  }

  /**
   * Server HTML template
   */
  removePermission(index) {
    const permissionArray = this.newAuthRole.$formGroup.get(['spec', 'permissions']) as FormArray;
    if (permissionArray.length > 1) {
      permissionArray.removeAt(index);
    }
  }
  /**
   * This API serves html template
   * @param $event
   * @param permission
   * @param permissionIndex
   */
  onGroupChange($event, permission, permissionIndex) {
    const selectedGroup = $event.value; // per p-dropdown API doc
  }
  /**
   * This API serves html template
   * @param $event
   * @param permission
   * @param permissionIndex
   */
  onKindChange($event, permission) {
    const value = $event.value; // per p-dropdown API doc
    // console.log(this.getClassName() + 'onKindChange()', value, permission);
  }
  /**
   * This API serves html template
   * Given a permission control, we compute the available options for resource-kind field
   */
  getKindOptions(permission: AbstractControl): SelectItem[] {
    const groupValue = permission.get('resource-group').value;
    const selectedGroup = Utility.makeFirstLetterUppercase(groupValue);
    const kinds = Utility.getKindsByCategory(selectedGroup);
    const kindsItems = Utility.stringArrayToSelectItem(kinds, false);
    return this.addAllKindItem(kindsItems);
  }

  /**
   * Role.permission.group/kind value combinations
   * We don’t want (group:all, kind:all), but we allow (group: Auth, kind:“_All_“).  Thus, add a "_All_"
   * @param selectItems
   */
  private addAllKindItem(selectItems: SelectItem[]): SelectItem[] {
    selectItems.push({ label: 'All', value: '_All_' });  // note: '_All_' is special in server.Auth
    return selectItems;
  }

}
