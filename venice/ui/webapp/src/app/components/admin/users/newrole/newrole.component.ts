import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, OnDestroy, OnChanges, ViewEncapsulation, SimpleChanges } from '@angular/core';
import { FormArray, Validators, FormGroup, AbstractControl, ValidatorFn, ValidationErrors, FormControl, FormControlName } from '@angular/forms';
import { required } from '@sdk/v1/utils/validators';
import { UsersComponent } from '../users.component';
import { Animations } from '@app/animations';
import { SelectItem } from 'primeng/primeng';
import { map, switchMap, tap, catchError } from 'rxjs/operators';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/generated/auth.service';
import { StagingService } from '@app/services/generated/staging.service';
import { AuthPermission, AuthPermission_actions_uihint, AuthPermission_actions } from '@sdk/v1/models/generated/auth';
import { AuthRoleBinding, AuthUser, AuthRole } from '@sdk/v1/models/generated/auth';
import { Utility } from '@app/common/Utility';
import { StagingBuffer, IStagingBuffer } from '@sdk/v1/models/generated/staging';
import { UIConfigsService } from '@app/services/uiconfigs.service';

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
 *
 *  role.spec.permission[i] (group and kind have inter-dependencies). Once a group is selected, kind options will chagne accordingly.
 *  It is like changing a state, the cities options will change.
 *  To prevent excessive computing kind options,  we introduce role.spec.permission[i][NewroleComponent.KINDOPTIONS]
 *  see this.onGroupChange($event, permission, index) and htmm template =>
 *  <p-dropdown class="newrole-severity" fxFlex="170px" formControlName="resource-kind" styleClass="newrole-font"
 *   [options]="permission.kindOptions" placeholder="kind..."  ></p-dropdown>
 *
 */
@Component({
  selector: 'app-newrole',
  templateUrl: './newrole.component.html',
  styleUrls: ['./newrole.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewroleComponent extends UsersComponent implements OnInit, OnDestroy, OnChanges {

  static KINDOPTIONS = 'kindOptions';
  static ACTIONOPTIONS = 'actionOptions';
  static RESOURCE_TENANT = null;
  static ACTIONOPTIONS_ALL = 'all-actions';
  static _ALL_ = '_All_';

  newAuthRole: AuthRole;

  @Input() selectedAuthRole: AuthRole;
  @Input() veniceRoles: AuthRole[] = [];  // hosted UI will pass in available AuthRoles
  @Output() formClose: EventEmitter<any> = new EventEmitter();
  groupOptions: SelectItem[] = Utility.convertCategoriesToSelectItem();

  constructor(protected _controllerService: ControllerService,
    protected _authService: AuthService,
    protected stagingService: StagingService,
    protected _uiconfigsService: UIConfigsService
  ) {
    super(_controllerService, _authService, stagingService, _uiconfigsService);
  }

  /**
   * override super.api()
   */
  getClassName(): string {
    return this.constructor.name;
  }

  ngOnInit() {
    // VS-209. add "ALL" option for group.
    this.groupOptions = this.addAllGroupOrKindItem(this.groupOptions);

  }

  setupData() {
    if (this.isEditMode()) {
      this.newAuthRole = this.getRoleFromSelectedRole();
    } else {
      if (!this.newAuthRole) {
        this.addEmptyPermission(); // VS-814, when adding a new role, user click refresh button, we should not add extra empty permission.
      }
      this.newAuthRole.$formGroup.get(['meta', 'name']).setValidators([
        this.newAuthRole.$formGroup.get(['meta', 'name']).validator,
        this.isRolenameValid(this.veniceRoles)]);
    }
  }

  isRolenameValid(authRoles: AuthRole[]): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      if (this.isRoleAlreadyExist(control.value, authRoles) || Utility.isEmpty(control.value)) {
        return {
          'role-name': {
            required: true,
            message: 'Role name is required and must be unique'
          }
        };
      }
      return null;
    };
  }

  getPermissionActionItem(element: any): any {
    let value = element;
    if (element.value) {    // VS- 465
      value = element.value;
    }
    const items = Utility.convertEnumToSelectItem(AuthPermission_actions_uihint).filter(item => {
      return (item.value === value);
    });
    return items[0];
  }

  /**
   * Take selectedAuthRole JSON value and make a new AuthRole object.
   *  We have to convert permissions[i].actions and permissions[i]['resource-names'] to UI friendly data structure
   */
  getRoleFromSelectedRole(newRole: AuthRole = null): AuthRole {
    if (newRole == null) {
      newRole = this.selectedAuthRole.getFormGroupValues() as AuthRole;
    }
    const permissions = newRole.spec.permissions;
    for (let i = 0; i < permissions.length; i++) {
      const actions = permissions[i].actions;

      // actions is like ["create" "update"], we want to change to [{label:value, value=create}, {label:update, value=update}]
      if (actions) {
        const actionsValues = [];
        actions.forEach((element) => {
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
    const newAuthRole = new AuthRole(newRole);
    // populate permission kind options
    const permissionControls = newAuthRole.$formGroup.get(['spec', 'permissions'])['controls'];
    for (let j = 0; j < permissionControls.length; j++) {
      permissionControls[j][NewroleComponent.KINDOPTIONS] = this.getKindOptions(permissionControls[j]);
      permissionControls[j][NewroleComponent.ACTIONOPTIONS] = this.getActionOptions(permissionControls[j]);
    }
    return newAuthRole;
  }

  ngOnChanges(changes: SimpleChanges) {
    this.setupData();
  }

  isEditMode(): boolean {
    return (this.selectedAuthRole) ? true : false;
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
    const formValid =  (hasFormGroupError === null);
    const permissionsValid = this.checkPermissions();
    return (formValid && permissionsValid);
  }

  /**
   * Serves HTML template.  User clicks [SAVE] button
   * @param $event
   */
  onSaveAddRole($event) {
    const isAllOK = this.isAllInputsValidated();
    if (isAllOK === true) {
      if (this.isEditMode()) {
        this.updateRole();
      } else {
        this.addRole();
      }
    } else {
      this._controllerService.invokeErrorToaster('Error', 'Input data is not valid');
    }
  }

  /**
   * Serves HTML template.  User clicks [CANCEL] button
   * @param $event
   */
  onCancelAddRole($event) {
    this.newAuthRole.$formGroup.reset();
    this.newAuthRole.setModelToBeFormGroupValues();
    this.selectedRolebindingsForUsers.length = 0;
    this.formClose.emit(false);
  }

  updateRole() {
    const newRole = Utility.TrimDefaultsAndEmptyFields(this.getRoleFromUI());
    this._authService.UpdateRole(newRole.meta.name, newRole).subscribe(
      response => {
        const createdRole: AuthRole = response.body as AuthRole;

        this._controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, 'Created Role ' + newRole.meta.name);
        this.formClose.emit(true);
      },
      this._controllerService.restErrorHandler(Utility.UPDATE_FAILED_SUMMARY)
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
    const newRole = Utility.TrimDefaultsAndEmptyFields(this.getRoleFromUI());

    const newRolebinding = new AuthRoleBinding();
    newRolebinding.meta.name = newRole.meta.name + '_binding';

    let buffername = null;

    this.createStagingBuffer().pipe( // Create buffer
      switchMap(responseBuffer => {
        const createdBuffer: StagingBuffer = responseBuffer.body as StagingBuffer;
        buffername = createdBuffer.meta.name;
        return this._authService.AddRole(newRole, buffername).pipe( // add role to buffer
          switchMap(responseAddRole => {
            const createdRole: AuthRole = responseAddRole.body as AuthRole;
            newRolebinding.spec.role = createdRole.meta.name;
            return this._authService.AddRoleBinding(newRolebinding, buffername).pipe( // add role binding to buffer
              switchMap(responseRoleBinding => {
                return this.commitStagingBuffer(buffername); // commit buffer
              })
            );
          }),
        );
      })
    ).subscribe(
      responseCommitBuffer => {
        this._controllerService.invokeSuccessToaster(Utility.CREATE_SUCCESS_SUMMARY, 'Created Role ' + newRole.meta.name + ' and Role-binding ' + newRolebinding.meta.name);
        this.formClose.emit(true);
      },
      error => {
        this._controllerService.invokeRESTErrorToaster(Utility.CREATE_FAILED_SUMMARY, error);
        this.deleteStagingBuffer(buffername, 'Failed to create role', false);
      }
    );
  }



  /**
   * Build Role create/update JSON data.
   * UI has special data structure in permission[i].actions and permissions[i]['resource-names']
   * We have to convert them to REST API complient data structure.
   */
  getRoleFromUI(): any {
    const newRole = Utility.getLodash().cloneDeep(this.newAuthRole.getFormGroupValues());
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
      // If kind should not have a group, we remove it now
      if (Utility.KINDS_WITHOUT_GROUP.includes(permissions[i]['resource-kind'])) {
        permissions[i]['resource-group'] = null;
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
    return new AuthRole(newRole, false);
  }

  /**
   * Server HTML template
   */
  addEmptyPermission() {
    const newPermission = new AuthPermission();
    newPermission['actions'] = []; // clear out action field's default value
    newPermission[NewroleComponent.KINDOPTIONS] = [];
    newPermission[NewroleComponent.ACTIONOPTIONS] = [];
    newPermission['resource-kind'] = '';
    newPermission['resource-group'] = '';
    if (this.newAuthRole == null) {
      const role = new AuthRole();
      role.spec.permissions.push(newPermission);
      this.newAuthRole = this.getRoleFromSelectedRole(role);
    } else {
      const permissionArray = this.newAuthRole.$formGroup.get(['spec', 'permissions']) as FormArray;
      permissionArray.insert(permissionArray.length, newPermission.$formGroup);
    }


  }

  /** adds default rollout permission as per VS-405. Commented out for now, though */
  /** addRolloutReadPermission() {
    const jsonPermission  =  {
      'resource-tenant': '',
      'resource-group': 'rollout',
      'resource-kind': 'Rollout',
      'resource-namespace': '',
      'actions': [
        AuthPermission_actions.read
      ]
    };
    const rolloutPermission = new AuthPermission(jsonPermission);
    const role = new AuthRole();
    role.spec.permissions.push(rolloutPermission);
    this.newAuthRole = this.getRoleFromSelectedRole(role);
  }
  */

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
    if (!permission[NewroleComponent.KINDOPTIONS]) {
      permission[NewroleComponent.KINDOPTIONS] = [];
    }
    permission[NewroleComponent.KINDOPTIONS].length = 0;
    // for VS-209, when group is "ALL", kind option must be "ALL"
    if ($event.value === NewroleComponent._ALL_) {
      permission[NewroleComponent.KINDOPTIONS] = [{ label: 'All', value: NewroleComponent._ALL_ }];
    } else {
      permission[NewroleComponent.KINDOPTIONS] = this.getKindOptions(permission);
    }
    // Resetting action and kind values
    permission.get('actions').setValue([]);
    permission.get('resource-kind').setValue('');
    permission[NewroleComponent.ACTIONOPTIONS] = this.getActionOptions(permission);
    // comment out permission.resource-tenant per VS-241 (GS-release)
    // this.setPermissionInputOnGroupChange($event.value, permission);
  }

  onKindChange($event, permission, permissionIndex) {
    permission[NewroleComponent.ACTIONOPTIONS] = this.getActionOptions(permission);
    // Clearing out previous actions
    // otherwise, if user goes from an object with all actions, checks one, and then
    // moves to a kind with no group, we will accidentally send the old value as well.
    // VS 576: Kinds without group have default action read
    if (Utility.KINDS_WITHOUT_GROUP.includes($event.value)) {
      permission.get('actions').setValue([{ label: 'Read', value: AuthPermission_actions.read }]);
    } else {
      permission.get('actions').setValue([]);
    }

  }

  setPermissionInputOnGroupChange(value: string, permission: FormControl) {
    if (this.isPermissionGroupNotOfTenantScope(value)) {
      // If it is not tenant scope, we set permission['resource-tenant'] to blank and disable data input;
      permission[NewroleComponent.RESOURCE_TENANT] = permission.get('resource-tenant').value;
      permission['controls']['resource-tenant'].setValue('');
      permission.get('resource-tenant').disable();
    } else {
      permission.get('resource-tenant').enable();  // enable data input.
      if (!Utility.isEmpty(permission[NewroleComponent.RESOURCE_TENANT])) {
        permission['controls']['resource-tenant'].setValue(permission[NewroleComponent.RESOURCE_TENANT]);
      }
    }
  }

  /**
   * A helper function to determine if selected permission group/kink value is of tenant scope.
   * @param groupValue
   * @param kindValue
   */
  isPermissionGroupNotOfTenantScope(groupValue: string, kindValue: string = null): boolean {
    // TODO: wait for code-gen code. This is hard coded.
    return (groupValue === 'cluster');
  }

  /**
   * This API serves html template
   * Given a permission control, we compute the available options for resource-kind field
   */
  getKindOptions(permission: AbstractControl): SelectItem[] {
    const groupValue = permission.get('resource-group').value;
    const selectedGroup = Utility.makeFirstLetterUppercase(groupValue);
    let kinds = Utility.getKindsByCategory(selectedGroup);
    if (selectedGroup === `Monitoring`) {
      // CategoryMapping has [AuditEvent, Event] under Monitoring group.
      // We add Fwlogs as well.
      kinds = Utility.getLodash().uniq(kinds.concat(Utility.KINDS_WITHOUT_GROUP));

      // VS-754 in appcontent.sidenav,ts we set auditevent, fwlog
      kinds = Utility.getLodash().dropRight(kinds, Utility.KINDS_WITHOUT_GROUP.length);
    }
    const kindsItems = Utility.stringArrayToSelectItem(kinds, false);
    return this.addAllGroupOrKindItem(kindsItems);
  }

  /**
   * This API serves html template
   * Given a permission control, we compute the available options for resource-kind field
   */
  getActionOptions(permission: AbstractControl): SelectItem[] {
    const selectedKind = permission.get('resource-kind').value;
    // If it's a kind without a group, only read permission can be given. Make sure 'read' is in lower
    if (Utility.KINDS_WITHOUT_GROUP.includes(selectedKind)) {
      return [{
        label: AuthPermission_actions_uihint.read,
        value: 'read'
      }];
    }
    if (permission.value['resource-group'] === 'staging') {
      return Utility.convertEnumToSelectItem(AuthPermission_actions_uihint);
    } else {
      return Utility.convertEnumToSelectItem(AuthPermission_actions_uihint, [AuthPermission_actions.clear, AuthPermission_actions.commit]);
    }
  }

  /**
   * Role.permission.group/kind value combinations
   * We don’t want (group:all, kind:all), but we allow (group: Auth, kind:“_All_“).  Thus, add a "_All_"
   * @param selectItems
   */
  addAllGroupOrKindItem(selectItems: SelectItem[]): SelectItem[] {
    let newSelectItems: SelectItem[] = [];
    newSelectItems.push({ label: 'All', value: NewroleComponent._ALL_ });  // note: '_All_' is special in server.Auth
    newSelectItems = newSelectItems.concat(selectItems);
    return newSelectItems;
  }

  onActionChange(event: any, permission: any, actionListboxWidget: any, permissionIndex: number) {
    const values = event.value;
    const selectedKind = permission.get('resource-kind').value;
    const index = this.getAllActionIndex(values);
    if (Utility.KINDS_WITHOUT_GROUP.includes(selectedKind)) {
      if (values.includes(NewroleComponent.ACTIONOPTIONS_ALL)) {
        values.remove(values.indexOf(NewroleComponent.ACTIONOPTIONS_ALL));
      }
      // VS 576: Kinds without group have default action Read
      // if (!values.includes({ label: 'read', value: AuthPermission_actions.read })) {
      if (values && values.length === 0) {
        values.push({ label: 'read', value: AuthPermission_actions.read });
      }
    }
    if (values.length > 1) {
      if (index !== -1 && values.length - 1 === index) {
        values[0] = values[index];
        values.splice(1);
      } else if (index !== -1 && values.length - 1 !== index) {
        values.splice(index, 1);
      }
      actionListboxWidget.value = values;
    }
  }


  checkPermissions(): boolean  {
    const newRole = this.newAuthRole;
    const permissions = newRole.spec.getFormGroupValues().permissions;
    for (let i = 0; i < permissions.length; i++) {
      const myPermission: AuthPermission = new AuthPermission(permissions[i]);
      const errors  = this.checkOnePermission(myPermission, i);
      if (errors.length > 0 ) {
        return false;
      }
    }
    return true;
  }

  checkOnePermission(permission: AuthPermission, index: number): string []  {
      const actions = permission.getFormGroupValues().actions;
      const errors: string [] = [];
      if (! (actions && actions.length > 0 ) ) {
        errors.push (  ' action list is empty');
      }
      if (Utility.isEmpty(permission.getFormGroupValues()['resource-group'])) {
        errors.push ( ' group is empty');
      }
      if (Utility.isEmpty(permission.getFormGroupValues()['resource-kind'])) {
        errors.push ( ' kind is empty');
      }
      if (errors.length > 0) {
        const _myErrors: string [] = [];
        _myErrors.push('Permission ' + index);
        _myErrors.concat(errors);
        return _myErrors;
      }
      return errors;
  }


  getAllActionIndex(values: any): number {
    return values.findIndex((value: SelectItem) => value.value === NewroleComponent.ACTIONOPTIONS_ALL);
  }

  displayActions(permission): boolean {
    if (permission.get('resource-kind').value !== '' && permission.get('resource-group').value !== '') {
      return true;
    }
    return false;
  }
}
