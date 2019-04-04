import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, OnDestroy, OnChanges, ViewEncapsulation, SimpleChanges } from '@angular/core';
import { FormArray, Validators, FormGroup, AbstractControl, ValidatorFn, ValidationErrors, FormControl } from '@angular/forms';
import { required } from '@sdk/v1/utils/validators';
import { UsersComponent } from '../users.component';
import { Animations } from '@app/animations';
import { SelectItem } from 'primeng/primeng';
import { map, switchMap, tap, catchError } from 'rxjs/operators';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/generated/auth.service';
import { StagingService } from '@app/services/generated/staging.service';
import { AuthPermission, AuthPermission_actions_uihint } from '@sdk/v1/models/generated/auth';
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
  static RESOURCE_TENANT = null;
  static ACTIONOPTIONS_ALL = 'AllActions';
  static _ALL_ = '_All_';

  newAuthRole: AuthRole;

  @Input() selectedAuthRole: AuthRole;
  @Input() veniceRoles: AuthRole[] = [];  // hosted UI will pass in available AuthRoles
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  groupOptions: SelectItem[] = Utility.convertCategoriesToSelectItem();

  permissionOptions: SelectItem[] = Utility.convertEnumToSelectItem(AuthPermission_actions_uihint);


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
  }

  setupData() {
    if (this.isEditMode()) {
      this.newAuthRole = this.getRoleFromSelectedRole();
    } else {
      this.newAuthRole = new AuthRole();
      this.newAuthRole.$formGroup.get(['meta', 'name']).setValidators([
        this.newAuthRole.$formGroup.get(['meta', 'name']).validator,
        this.isRolenameValid(this.veniceRoles)]);

      const permissions = this.newAuthRole.$formGroup.get(['spec', 'permissions']) as FormArray;
      if (permissions.length === 0) {
        this.addPermission();
      }
    }
  }

  isRolenameValid(authRoles: AuthRole[]): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      if (this.isRoleAlreadyExist(control.value, authRoles)) {
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
    const newAuthRole = new AuthRole(newRole);
    // populate permission kind options
    const permissionControls = newAuthRole.$formGroup.get(['spec', 'permissions'])['controls'];
    for (let j = 0; j < permissionControls.length; j++) {
      permissionControls[j][NewroleComponent.KINDOPTIONS] = this.getKindOptions(permissionControls[j]);
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
    return new AuthRole(newRole, false);
  }

  /**
   * Server HTML template
   */
  addPermission() {
    const permissionArray = this.newAuthRole.$formGroup.get(['spec', 'permissions']) as FormArray;
    const newPermission = new AuthPermission();
    newPermission['actions'] = []; // clear out action field's default value
    newPermission[NewroleComponent.KINDOPTIONS] = [];
    permissionArray.insert(0, newPermission.$formGroup);
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
    if (!permission[NewroleComponent.KINDOPTIONS]) {
      permission[NewroleComponent.KINDOPTIONS] = [];
    }
    permission[NewroleComponent.KINDOPTIONS].length = 0;
    permission[NewroleComponent.KINDOPTIONS] = this.getKindOptions(permission);

    this.setPermissionInputOnGroupChange($event.value, permission);
  }

  setPermissionInputOnGroupChange(value: string , permission: FormControl) {
    if (this.isPermissionGroupNotOfTenantScope(value)) {
      // If it is not tenant scope, we set permission['resource-tenant'] to blank and disable data input;
      permission[NewroleComponent.RESOURCE_TENANT] = permission.get('resource-tenant').value;
      permission['controls']['resource-tenant'].setValue('');
      permission.get('resource-tenant').disable();
    } else {
      permission.get('resource-tenant').enable();  // enable data input.
      if ( !Utility.isEmpty(permission[NewroleComponent.RESOURCE_TENANT])) {
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
    return (groupValue === 'cluster') ;
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
    selectItems.push({ label: 'All', value: NewroleComponent._ALL_ });  // note: '_All_' is special in server.Auth
    return selectItems;
  }

  onActionChange(event: any, permission: any, actionListboxWidget: any, permissionIndex: number) {
    const values = event.value;
    if (values.length > 1) {
      const index = this.getAllActionIndex(values);
      if (index !== -1) {
        values.splice(index, 1);
        actionListboxWidget.value = values;
        this.permissionOptions = Utility.convertEnumToSelectItem(AuthPermission_actions_uihint, [NewroleComponent.ACTIONOPTIONS_ALL]);
      }
    } else if (values.length === 1) {  // there is only one option selected
      const index = this.getAllActionIndex(values);
      if (index === -1) {
        // NewroleComponent.ACTIONOTIONT_ALL is NOT the only selected option. we take out
        this.permissionOptions = Utility.convertEnumToSelectItem(AuthPermission_actions_uihint, [NewroleComponent.ACTIONOPTIONS_ALL]);
      }
    } else {
      this.permissionOptions = Utility.convertEnumToSelectItem(AuthPermission_actions_uihint);
    }

  }


  private getAllActionIndex(values: any): number {
    return values.findIndex((value: SelectItem) => value.value === NewroleComponent.ACTIONOPTIONS_ALL);
  }
}
