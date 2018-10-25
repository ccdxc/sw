import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, OnDestroy, OnChanges, ViewEncapsulation } from '@angular/core';
import { Validators } from '@angular/forms';

import { UsersComponent } from '../users.component';
import { ErrorStateMatcher } from '@angular/material';
import { Animations } from '@app/animations';
import { SelectItem, MessageService } from 'primeng/primeng';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/generated/auth.service';
import { AuthRoleBinding, AuthUser } from '@sdk/v1/models/generated/auth';


@Component({
  selector: 'app-newuser',
  templateUrl: './newuser.component.html',
  styleUrls: ['./newuser.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewuserComponent extends UsersComponent implements OnInit, AfterViewInit, OnChanges, OnDestroy {

  newAuthUser: AuthUser;
  selectedRolebindings: any[] = [];

  @Input() authRoleBindings: AuthRoleBinding[] = [];
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  @Input() rolebindingOptions: SelectItem[] = [];

  errorChecker = new ErrorStateMatcher();

  protected rolebindingUpdateMap = {};

  constructor(protected _controllerService: ControllerService,
    protected _authService: AuthService,
    protected messageService: MessageService
  ) {
    super(_controllerService, _authService, messageService);
  }

  ngOnInit() {
    this.newAuthUser = new AuthUser();
    this.newAuthUser.$formGroup.get(['meta', 'name']).setValidators(Validators.required);
    this.newAuthUser.$formGroup.get(['spec', 'fullname']).setValidators(Validators.required);
    this.newAuthUser.$formGroup.get(['spec', 'password']).setValidators(Validators.required);
    this.newAuthUser.$formGroup.get(['spec', 'type']).setValidators(Validators.required);
    this.newAuthUser.$formGroup.get(['spec', 'email']).setValidators(Validators.required);
  }

  ngAfterViewInit() {

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
    this.selectedRolebindings.length = 0;
  }

  getClassName(): string {
    return this.constructor.name;
  }

  addUser() {
    const newUser = this.newAuthUser.getFormGroupValues();
    this._authService.AddUser(newUser).subscribe(
      response => {
        const createdUser: AuthUser = response.body as AuthUser;

        this.invokeSuccessToaster('Creation Successful', 'Created user ' + newUser.meta.name);
        this.handleAddUpdateUserRESTCallSuccess(createdUser);
      },
      this.restErrorHandler('Create User Failed')
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
    const errors = this.getAllFormgroupErrors(this.newAuthUser.$formGroup);
    if (errors === null) {
      this.addUser();
    }
  }

  onCancelAddUser($event) {
    console.log(this.getClassName() + '.onCancelAddUser()');
    this.newAuthUser.$formGroup.reset();
    this.selectedRolebindings.length = 0;
    this.formClose.emit(false);
  }

  onRolebindingChange($event) {
    console.log(this.getClassName() + '.onRolebindingChange()');
    this.selectedRolebindings = $event.value;
  }

  isAllInputsValidated() {
    const hasFormGroupError = this.getAllFormgroupErrors(this.newAuthUser.$formGroup);
    const hasSelectedRole = (this.selectedRolebindings && this.selectedRolebindings.length > 0);
    return (hasFormGroupError === null) && (hasSelectedRole);
  }


}
