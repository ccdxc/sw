import { Component, OnInit, ViewEncapsulation, Output, EventEmitter, Input, ViewChild, ViewChildren, SimpleChanges, OnChanges } from '@angular/core';
import { AuthpolicybaseComponent } from '@app/components/settings-group/authpolicy/authpolicybase/authpolicybase.component';
import { Animations } from '@app/animations';
import { AuthLdap, AuthLdapServer } from '@sdk/v1/models/generated/auth';
import { FormArray, FormControl, AbstractControl } from '@angular/forms';
import { MatSlideToggleChange } from '@angular/material';

import { LDAPCheckResponse, LDAPCheckType, CheckResponseError } from '@app/components/settings-group/authpolicy/.';
import { AuthPolicyUtil } from '@app/components/settings-group/authpolicy/AuthPolicyUtil';
import { Utility } from '@app/common/Utility';
/**
 * LdapComponent (child) is a child component of AuthPolicy.component (parent)
 * parent passes in "LDAPData" to child, child has a "LDAPObject" to reflect "LDAPData".
 *
 * When user wants to save LDAP, test BIND or test LDAP server connection, LdapComponent emits event to parent and let parent handles the REST calls.
 *
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
export class LdapComponent extends AuthpolicybaseComponent implements OnInit, OnChanges {
  isHover: boolean = false;
  LDAPEditMode: boolean = false;
  inCreateMode: boolean = false;
  LDAPObject: AuthLdap = new AuthLdap();

  verifyCertToggleFormArray: FormArray;

  @Input() LDAPData: AuthLdap;
  @Input() ldapBindCheckResponse: LDAPCheckResponse = null;
  @Input() ldapConnCheckResponse: LDAPCheckResponse = null;
  @Output() invokeCheckLDAPServerConnect: EventEmitter<any> = new EventEmitter();
  @Output() invokeCheckLDAPBindConnect: EventEmitter<any> = new EventEmitter();
  @Output() invokeSaveLDAP: EventEmitter<any> = new EventEmitter();

  constructor() {
    super();
  }

  getClassName(): string {
    return this.constructor.name;
  }

  ngOnInit() {
    this.updateLDAPObject();
  }

  ngOnChanges(changes: SimpleChanges) {
    this.updateLDAPObject();
  }

  updateLDAPObject() {
    this.LDAPObject.setValues(this.LDAPData);
  }

  updateLDAPData() {
    if (this.LDAPObject) {
      this.LDAPData.setValues(this.LDAPObject.getFormGroupValues());
    }
  }

  toggleEdit() {
    this.LDAPEditMode = !this.LDAPEditMode;
    if (this.LDAPEditMode) {
      this.verifyCertToggleFormArray = new FormArray([]);
      // Add a blank server if there is none
      if (this.LDAPObject.servers.length === 0) {
        const serversLDAP = this.LDAPObject.$formGroup.get('servers') as FormArray;
        serversLDAP.insert(0, new AuthLdapServer({ 'tls-options': { 'start-tls': true, 'skip-server-cert-verification': false } }).$formGroup);
      }
      const servers = this.LDAPObject.$formGroup.get('servers') as FormArray;
      // Each server, we register the verify toggle, and check for diasbling fields
      servers.controls.forEach((server, index) => {
        const verifyCert = !server.get(['tls-options', 'skip-server-cert-verification']).value;
        this.verifyCertToggleFormArray.insert(index, new FormControl(verifyCert));
        this.checkServerTlsDisabling(server, index);
      });
    }
  }

  // For all servers, if start-tls is disabled, we disable the other tls fields
  // if verify serve is disabled, we disable server-name and trusted-cert fields are disabled
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
    this.checkServerTlsDisabling(server, index);
  }

  addServer() {
    const servers = this.LDAPObject.$formGroup.get('servers') as FormArray;
    servers.insert(0, new AuthLdapServer({ 'tls-options': { 'start-tls': true, 'skip-server-cert-verification': false } }).$formGroup);
    this.verifyCertToggleFormArray.insert(0, new FormControl(true));
  }

  removeServer(index) {
    const servers = this.LDAPObject.$formGroup.get('servers') as FormArray;
    if (servers.length > 1) {
      servers.removeAt(index);
      this.verifyCertToggleFormArray.removeAt(index);
    }
  }

  cancelEdit() {
    this.LDAPEditMode = false;
    if (this.inCreateMode) {
      // create form is canceling,
      // Remove the data we added
      this.LDAPData = null;
    }
    // Reset the LDAPObject with the passed in data
    this.updateLDAPObject();
  }

  saveLDAP() {
    this.updateLDAPData();
    this.invokeSaveLDAP.emit(false); // emit event to parent to update LDAP
    // POST DATA
    this.LDAPEditMode = false;
    // Reset the LDAPObject with the passed in data
    this.updateLDAPObject();
  }

  createLDAP() {
    this.invokeSaveLDAP.emit(true);  // emit event to parent to create LDAP
    this.LDAPData = new AuthLdap();
    this.inCreateMode = true;
    this.LDAPEditMode = true;
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
  hasBindConfigError(checkResponse: LDAPCheckResponse): boolean {
    const bindCheckResponseError = this.hasConfigErrorHelper(this.ldapBindCheckResponse);
    return (bindCheckResponseError.errors.length > 0);
  }

  hasConnConfigError(checkResponse: LDAPCheckResponse): boolean {
    const connCheckResponseError = this.hasConfigErrorHelper(this.ldapConnCheckResponse);
    return (connCheckResponseError.errors.length > 0);
  }

  /**
   * Helper function.
   */
  hasConfigErrorHelper(checkResponse: LDAPCheckResponse): CheckResponseError {
    return AuthPolicyUtil.processLDAPCheckResponse(checkResponse);  // invoke index.ts function
  }

  onBindErrorMessageClick($event) {
    const msgs = this.getErrorMessages(this.ldapBindCheckResponse);
    alert(JSON.stringify(Object.values(msgs)));
  }

  onConnErrorMessageClick($event) {
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
}
