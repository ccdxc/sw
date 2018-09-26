import { Component, OnInit, ViewEncapsulation, Output, EventEmitter, Input, ViewChild, ViewChildren, SimpleChanges, OnChanges } from '@angular/core';
import { AuthpolicybaseComponent } from '@app/components/settings-group/authpolicy/authpolicybase/authpolicybase.component';
import { Animations } from '@app/animations';
import { IAuthLdap, AuthLdap, AuthLdapServer } from '@sdk/v1/models/generated/auth';
import { FormArray, FormControl, AbstractControl } from '@angular/forms';
import { MatSlideToggleChange } from '@angular/material';

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

  @Input() LDAPData: IAuthLdap;


  constructor() {
    super();
  }

  ngOnInit() {
    this.updateLDAPObject();
  }

  ngOnChanges(changes: SimpleChanges) {
    this.updateLDAPObject();
  }

  updateLDAPObject() {
    this.LDAPObject.setValues(this.LDAPData);
    this.LDAPObject.setFormGroupValues();
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
    // POST DATA
    this.LDAPEditMode = false;
    // Reset the LDAPObject with the passed in data
    this.updateLDAPObject();
  }

  createLDAP() {
    this.LDAPData = new AuthLdap();
    this.inCreateMode = true;
    this.LDAPEditMode = true;
  }
}
