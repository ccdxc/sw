import { Component, OnInit, ViewEncapsulation, Output, EventEmitter, Input, ViewChild, ViewChildren, SimpleChanges, OnChanges, AfterViewInit, AfterContentInit } from '@angular/core';
import { ValidatorFn, ValidationErrors } from '@angular/forms';
import { AuthpolicybaseComponent } from '@app/components/admin/authpolicy/authpolicybase/authpolicybase.component';
import { Animations } from '@app/animations';
import { AuthLdap, AuthLdapDomain, AuthLdapServer, AuthAuthenticationPolicy } from '@sdk/v1/models/generated/auth';
import { required } from '@sdk/v1/utils/validators';
import { FormArray, FormControl, AbstractControl } from '@angular/forms';
import { MatSlideToggleChange } from '@angular/material';

import { LDAPCheckResponse, LDAPCheckType, LdapSave, CheckResponseError } from '@app/components/admin/authpolicy/.';
import { AuthPolicyUtil } from '@app/components/admin/authpolicy/AuthPolicyUtil';
import { Utility } from '@app/common/Utility';
import { UIConfigsService } from '@app/services/uiconfigs.service';

import { ControllerService } from '@app/services/controller.service';

/**
 * LdapComponent (child) is a child component of AuthPolicy.component (parent)
 * parent passes in "LDAPData" to child, child has a "LDAPObject" to reflect "LDAPData".
 *
 * When user wants to save LDAP, test BIND or test LDAP server connection, LdapComponent emits event to parent and let parent handles the REST calls.
 *
 * "LDAPEditMode" is used to control switching view/edit UI.  When it is non-edit mode, mat-slider widget will be disabled.
 *
 * Validation:
 * 2019-01-24, if we change auth.proto  validation settings, it will cause a lot of go tests failures.
 * We set validation in UI instead.  see setLDAPValidationRules() API
 */
@Component({
  selector: 'app-ldap',
  templateUrl: './ldap.component.html',
  styleUrls: ['./ldap.component.scss'],
  encapsulation: ViewEncapsulation.None,
  host: {
    '(mouseenter)': 'onMouseEnter()',
    '(mouseleave)': 'onMouseLeave()'
  },
  animations: [Animations]
})

export class LdapComponent extends AuthpolicybaseComponent implements OnInit, OnChanges, AfterContentInit {
  isHover: boolean = false;
  LDAPEditMode: boolean = false;
  inCreateMode: boolean = false;
  LDAPObject: AuthLdap = new AuthLdap();

  verifyCertToggleFormArray: FormArray;

  @Input() LDAPData: AuthLdap;
  @Input() ldapBindCheckResponse: LDAPCheckResponse = null;
  @Input() ldapConnCheckResponse: LDAPCheckResponse = null;
  @Output() invokeCheckLDAPServerConnect: EventEmitter<AuthLdap> = new EventEmitter();
  @Output() invokeCheckLDAPBindConnect: EventEmitter<AuthLdap> = new EventEmitter();
  @Output() invokeSaveLDAP: EventEmitter<LdapSave> = new EventEmitter();
  @Output() invokeCreateLDAP: EventEmitter<LdapSave> = new EventEmitter();
  @Output() invokeRemoveLDAP: EventEmitter<AuthLdap> = new EventEmitter();

  @Input() parentAuthPolicy: AuthAuthenticationPolicy = null;

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService) {
    super(_controllerService, uiconfigsService);
  }

  getClassName(): string {
    return this.constructor.name;
  }

  ngOnInit() {
    super.ngOnInit();
    this.setLDAPEditMode(true);
  }

  ngAfterContentInit() {
    this.updateLDAPObject();
    this.setLDAPEditMode(false); // set LDAPEditMode to false, UI will disable LDAD-enabled slider widget
  }

  ngOnChanges(changes: SimpleChanges) {
    this.updateLDAPObject();
  }

  updateLDAPObject() {
    this.LDAPObject.setValues(this.LDAPData);
    if (this.LDAPObject.domains == null || this.LDAPObject.domains.length === 0) {
      const newDomain = new AuthLdapDomain();
      this.LDAPObject.domains = [newDomain];
      const domains = this.LDAPObject.$formGroup.get(['domains']) as FormArray;
      domains.insert(0, newDomain.$formGroup);
    }
    this.setLDAPValidationRules();
  }

  /**
   * This function is responsible for setting Required Validation on form fields
   */
  private setLDAPValidationRules() {
    // TODO: Changing auth.proto LDAP validation rule has ripple effects on go-test, we set validator in UI 2019-01-24
    this.LDAPObject.$formGroup.get(['domains', '0', 'bind-dn']).setValidators(required);
    this.LDAPObject.$formGroup.get(['domains', '0', 'base-dn']).setValidators(required);
    if (this.inCreateMode) {
      this.LDAPObject.$formGroup.get(['bind-password']).setValidators(required);
    }

    const attribute_mapping: AbstractControl = this.LDAPObject.$formGroup.get(['domains', '0', 'attribute-mapping']) as AbstractControl;
    attribute_mapping.get('user-object-class').setValidators(required);
    attribute_mapping.get('user').setValidators(required);
    attribute_mapping.get('group-object-class').setValidators(required);
    attribute_mapping.get('group').setValidators(required);

    const servers: FormArray = this.LDAPObject.$formGroup.get('domains.0.servers') as FormArray;
    if (servers.length > 0) {
      const controls = servers.controls;
      for (let i = 0; i < controls.length; i++) {
        const control: AbstractControl = controls[i];
        this.setValidatorOnServerControl(control);
      }
    }
  }
  private setValidatorOnServerControl(control: AbstractControl) {
    const controlTLSOption = control.get(['tls-options']);
    control.get(['tls-options', 'server-name']).setValidators(this.isTLSOptionFieldRequired(controlTLSOption, 'server-name'));
    control.get(['tls-options', 'trusted-certs']).setValidators(this.isTLSOptionFieldRequired(controlTLSOption, 'trusted-certs'));
  }

  /**
   * TLSOptions trust-certs is not required if Skip-Server-CertVerification is set to true.
   * TLSOptions server name is not required if Skip-Server-CertVerification is set to true.
   * @param controlTLSOption
   */
  private isTLSOptionFieldRequired(controlTLSOption: AbstractControl, fieldname: string): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      if (controlTLSOption.get(['skip-server-cert-verification']).value === false && Utility.isEmpty(controlTLSOption.get([fieldname]).value)) {
        return {
          required: {
            'required': true,
            'message': 'This field is required if skip-server-cert-verification set to false'
          }
        };
      }
      return null;
    };
  }

  updateLDAPData() {
    if (this.LDAPObject) {
      this.LDAPData.setValues(this.LDAPObject.getFormGroupValues());
    }
  }

  /**
   * Helps in toggling between edit and view mode.
   * Calls the setLDAPValidationRules() whenever the view switches to edit mode.
   */
  toggleEdit() {
    this.setLDAPEditMode(!this.LDAPEditMode);
    if (this.LDAPEditMode) {
      this.verifyCertToggleFormArray = new FormArray([]);
      // Add a blank server if there is none
      if (this.LDAPObject.domains[0].servers.length === 0) {
        this.addServer();
      }
      this.setLDAPValidationRules();
      const servers = this.LDAPObject.$formGroup.get(['domains', '0', 'servers']) as FormArray;
      // Each server, we register the verify toggle, and check for diasbling fields
      servers.controls.forEach((server, index) => {
        const verifyCert = !server.get(['tls-options', 'skip-server-cert-verification']).value;
        this.verifyCertToggleFormArray.insert(index, new FormControl(verifyCert));
        this.checkServerTlsDisabling(server, index);
      });
    }

  }

  /**
   * Responsible for deleting selected LDAP policy
   */
  onDeleteLDAP() {
    this._controllerService.invokeConfirm({
      header: Utility.generateDeleteConfirmMsg('Config', 'LDAP'),
      message: 'This action cannot be reversed',
      acceptLabel: 'Delete',
      accept: () => {
        this.invokeRemoveLDAP.emit(this.LDAPData);
      }
    });
  }


  setLDAPEditMode(isInEditMode) {
    this.LDAPEditMode = isInEditMode;
  }

  /**
   * This function controls the disabling and enabling of TLS fields.
   * For all servers:
   * 1. if start-tls is disabled, the other tls fields are disabled.
   * 2. if verify serve is disabled, server-name and trusted-cert fields are disabled.
   */
  checkServerTlsDisabling(server: AbstractControl, index: number) {
    const tlsOptions = server.get('tls-options');
    if (!tlsOptions.value['start-tls']) {
      tlsOptions.get('server-name').disable();
      this.verifyCertToggleFormArray.at(index).disable();
      tlsOptions.get('trusted-certs').disable();
    } else if (tlsOptions.value['skip-server-cert-verification']) {
      this.verifyCertToggleFormArray.at(index).enable();
      tlsOptions.get('server-name').disable();
      tlsOptions.get('trusted-certs').disable();
    } else {
      tlsOptions.get('server-name').enable();
      this.verifyCertToggleFormArray.at(index).enable();
      tlsOptions.get('trusted-certs').enable();
    }
  }

  toggleStartTls(server, index, event: MatSlideToggleChange) {
    this.checkServerTlsDisabling(server, index);
  }

  // UI has this field inverted from the model
  // true toggle on the UI means this field should be false.
  toggleSkipVerification(server: FormControl, index: number, event: MatSlideToggleChange) {
    server.get(['tls-options', 'skip-server-cert-verification']).setValue(!event.checked);
    // once toggling the slide, we want to re-run the validation
    server.get(['tls-options', 'server-name']).updateValueAndValidity();
    server.get(['tls-options', 'trusted-certs']).updateValueAndValidity();
    if (!event.checked) {
      server.get(['tls-options', 'server-name']).disable();
      server.get(['tls-options', 'trusted-certs']).disable();
    } else {
      server.get(['tls-options', 'server-name']).enable();
      server.get(['tls-options', 'trusted-certs']).enable();
    }
    // TODO: comment out this line "this.checkServerTlsDisabling(server, index);" // per pull/8528 comment  -> allow the trusted-certs be filled out while skip verification is true.
  }

  addServer() {
    const servers = this.LDAPObject.$formGroup.get(['domains', '0', 'servers']) as FormArray;
    const newServer = new AuthLdapServer({ 'tls-options': { 'start-tls': true, 'skip-server-cert-verification': false } }).$formGroup;
    servers.insert(0, newServer);
    this.verifyCertToggleFormArray.insert(0, new FormControl(true));

    setTimeout(() => {
      // TODO: have to use a timer here to avoid angular firing up error on state inconsistency
      this.setValidatorOnServerControl(newServer);
    }, 1000);
  }

  removeServer(index) {
    const servers = this.LDAPObject.$formGroup.get(['domains', '0', 'servers']) as FormArray;
    if (servers.length > 1) {
      servers.removeAt(index);
      this.verifyCertToggleFormArray.removeAt(index);
    }
  }

  /**
   * Using this function, user returns back to view mode.
   * If this is called during creation, then all data is deleted i.e. form group gets reset.
   * Else the previous data that was there before this editing is preserved
   */
  cancelEdit() {
    this.setLDAPEditMode(false); // restore LDAPEditMode to false
    if (this.inCreateMode) {
      // create form is canceling,
      // Remove the data we added
      this.LDAPData = null;
      this.LDAPObject.$formGroup.reset();
    }
    // Reset the LDAPObject with the passed in data
    this.updateLDAPObject();
    this.inCreateMode = false;
  }

  /**
   * This function can be only called when all form validation rules are met
   * during creation or updation. Emits LDAP data to the parent component,
   * responsible for REST calls, to save the object.
   */
  saveLDAP() {
    this.updateLDAPData();
    let ldapSave: LdapSave;
    if (this.inCreateMode) {
      if (this.isAllInputsValid(this.LDAPObject)) {
        ldapSave = {
          createData: this.LDAPData,
          onSuccess: (resp) => { this.setLDAPEditMode(false); },
        };
        this.invokeCreateLDAP.emit(ldapSave);
      } else {
        this._controllerService.invokeErrorToaster('Invalid', 'There are invalid inputs.  Fields with "*" are requried');
      }
    } else {
      // POST DATA
      ldapSave = {
        onSuccess: (resp) => { this.setLDAPEditMode(false); },
      };
      this.invokeSaveLDAP.emit(ldapSave); // emit event to parent to update LDAP if REST call succeeds, ngOnChange() will bb invoked and refresh data.
    }
  }

  /**
   * @param authLDAP
   * Checks whether the form group data satisfies the validation rules
   */
  isAllInputsValid(authLDAP: AuthLdap): boolean {
    return (!Utility.getAllFormgroupErrors(authLDAP.$formGroup));
  }

  /**
   * This function sets new form for creation of the LDAP policy
   */
  createLDAP() {
    this.LDAPData = new AuthLdap();
    this.toggleEdit();
    this.inCreateMode = true;
    this.setLDAPEditMode(true);
  }

  onCheckLdapConnection($event) {
    this.updateLDAPData();
    this.ldapConnCheckResponse = null;
    this.invokeCheckLDAPServerConnect.emit(this.LDAPData);
  }

  onCheckLDAPBindConnect($event) {
    this.updateLDAPData();
    this.ldapBindCheckResponse = null;
    this.invokeCheckLDAPBindConnect.emit(this.LDAPData);
  }

  /**
  * This api serves API
  */
  hasBindConfigError(): boolean {
    const bindCheckResponseError = this.hasConfigErrorHelper(this.ldapBindCheckResponse);
    return (bindCheckResponseError.errors.length > 0);
  }

  hasConnConfigError(): boolean {
    const connCheckResponseError = this.hasConfigErrorHelper(this.ldapConnCheckResponse);
    return (connCheckResponseError.errors.length > 0);
  }

  /**
   * Helper function.
   */
  hasConfigErrorHelper(checkResponse: LDAPCheckResponse): CheckResponseError {
    return AuthPolicyUtil.processLDAPCheckResponse(checkResponse);  // invoke index.ts function
  }

  onBindErrorMessageClick() {
    const msgs = this.getErrorMessages(this.ldapBindCheckResponse);
    alert(JSON.stringify(Object.values(msgs)));
  }

  onConnErrorMessageClick() {
    const msgs = this.getErrorMessages(this.ldapConnCheckResponse);
    alert(JSON.stringify(Object.values(msgs)));
  }

  getErrorMessages(checkResponse: LDAPCheckResponse): any {
    const connCheckResponseError = this.hasConfigErrorHelper(checkResponse);
    const _ = Utility.getLodash();
    const msgs = _.mapValues(connCheckResponseError.errors, 'message');
    return msgs;
  }

  getConnConfigErrorTooltip(): string {
    const msgs = this.getErrorMessages(this.ldapConnCheckResponse);
    return Object.values(msgs).join('\n');
  }

  getBindConfigErrorTooltip(): string {
    const msgs = this.getErrorMessages(this.ldapBindCheckResponse);
    return Object.values(msgs).join('\n');
  }

  /**
   * This api serves html template. It controlls whether to enble "SAVE" button
   */
  enableSaveButton(): boolean {
    return this.isAllInputsValid(this.LDAPObject);
  }


}
