import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { SharedModule } from '@app/components/shared/shared.module';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';

import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { By } from '@angular/platform-browser';
import { MessageService } from '@app/services/message.service';
import { PrimengModule } from '@app/lib/primeng.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { AuthLdap } from '@sdk/v1/models/generated/auth';

import { TestingUtility } from '@app/common/TestingUtility';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { AuthService } from '@app/services/auth.service';
import { LdapComponent } from './ldap.component';

describe('LdapComponent', () => {
  let component: LdapComponent;
  let fixture: ComponentFixture<LdapComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [LdapComponent],
      imports: [
        MaterialdesignModule,
        RouterTestingModule,
        HttpClientTestingModule,
        SharedModule,
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        PrimengModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService,
        UIConfigsService,
        LicenseService,
        AuthService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(LdapComponent);
    component = fixture.componentInstance;
  });

  it('handle data conflict while in edit mode', () => {
    // TODO
  });

  it('should display toggle and values based on input', () => {
    component.LDAPData = new AuthLdap({
      enabled: true,
      domains: [
      {
      'base-dn': 'basedn',
      'bind-dn': 'binddn',
      'bind-password': 'bindpass',
      'attribute-mapping': {
        'email': 'email',
        'fullname': 'fullname',
        'user': 'user',
        'user-object-class': 'user-obj',
        'group-object-class': 'group-obj-class',
        'group': 'group',
        'tenant': 'tenant'
      },
      servers: [
        {
          'url': '10.1.1.10:8000',
          'tls-options': {
            'server-name': 'server1',
            'skip-server-cert-verification': false,
            'start-tls': true,
            'trusted-certs': 'example cert'
          }
        },
        {
          'url': '10.1.1.11:8000',
          'tls-options': {
            'server-name': 'server1',
            'skip-server-cert-verification': true,
            'start-tls': false,
            'trusted-certs': 'example cert'
          }
        }
      ]
      }
      ]
    });
    fixture.detectChanges();
    let toggle = fixture.debugElement.queryAll(By.css('.mat-checked'));
    expect(toggle.length).toBe(0);

    let values = fixture.debugElement.queryAll(By.css('.ldap-input'));
    expect(values[0].nativeElement.innerText).toContain('binddn');
    expect(values[1].nativeElement.innerText).toContain('**********');
    expect(values[2].nativeElement.innerText).toContain('basedn');
    expect(values[3].nativeElement.innerText).toContain('user-obj');
    expect(values[4].nativeElement.innerText).toContain('group-obj-class');
    expect(values[5].nativeElement.innerText).toContain('user');
    expect(values[6].nativeElement.innerText).toContain('group');
    expect(values[7].nativeElement.innerText).toContain('tenant');
    expect(values[8].nativeElement.innerText).toContain('fullname');
    expect(values[9].nativeElement.innerText).toContain('email');

    // server view - lock should only be lock icon if start-tls is true
    // and skip-server-cert-verification is false
    let servers = fixture.debugElement.query(By.css('.ldap-server-viewmode'));
    expect(servers).toBeTruthy();
    expect(servers.children.length).toBe(4);
    let icons = servers.children[2].queryAll(By.css('mat-icon'));
    expect(icons.length).toBe(1);
    expect(icons[0].nativeElement.innerText).toBe('lock');
    expect(servers.children[2].children[1].nativeElement.innerText).toContain('10.1.1.10:8000');

    icons = servers.children[3].queryAll(By.css('mat-icon'));
    expect(icons[0].nativeElement.innerText).toBe('lock_open');
    expect(servers.children[3].children[1].nativeElement.innerText).toContain('10.1.1.11:8000');

    // test new incoming data updates the view
    component.LDAPData = new AuthLdap({
      enabled: false,
      domains: [
        {
          'base-dn': 'basedn-change',
          'bind-dn': 'binddn',
          'bind-password': 'bindpass',
          'attribute-mapping': {
            'email': 'email',
            'fullname': 'fullname-change',
            'user': 'user',
            'user-object-class': 'user-obj-change',
            'group-object-class': 'group-obj-class',
            'group': 'group',
            'tenant': 'tenant'
          },
          servers: [
            {
              'url': '10.1.1.10:8000',
              'tls-options': {
                'server-name': 'server1',
                'skip-server-cert-verification': true,
                'start-tls': true,
                'trusted-certs': 'example cert'
              }
            },
            {
              'url': '10.10.10.11:8000',
              'tls-options': {
                'server-name': 'server1',
                'skip-server-cert-verification': true,
                'start-tls': false,
                'trusted-certs': 'example cert'
              }
            }
          ]
        }
      ]
    });
    component.ngOnChanges(null);
    fixture.detectChanges();
    toggle = fixture.debugElement.queryAll(By.css('.mat-checked'));
    expect(toggle.length).toBe(0);

    values = fixture.debugElement.queryAll(By.css('.ldap-input'));
    expect(values[0].nativeElement.innerText).toContain('binddn');
    expect(values[1].nativeElement.innerText).toContain('**********');
    expect(values[2].nativeElement.innerText).toContain('basedn-change');
    expect(values[3].nativeElement.innerText).toContain('user-obj-change');
    expect(values[4].nativeElement.innerText).toContain('group-obj');
    expect(values[5].nativeElement.innerText).toContain('user');
    expect(values[6].nativeElement.innerText).toContain('group');
    expect(values[7].nativeElement.innerText).toContain('tenant');
    expect(values[8].nativeElement.innerText).toContain('fullname-change');
    expect(values[9].nativeElement.innerText).toContain('email');

    // server view - lock should only be lock icon if start-tls is true
    // and skip-server-cert-verification is false
    servers = fixture.debugElement.query(By.css('.ldap-server-viewmode'));
    expect(servers).toBeTruthy();
    expect(servers.children.length).toBe(4);
    icons = servers.children[2].queryAll(By.css('mat-icon'));
    expect(icons.length).toBe(1);
    expect(icons[0].nativeElement.innerText).toBe('lock_open');
    expect(servers.children[2].children[1].nativeElement.innerText).toContain('10.1.1.10:8000');

    icons = servers.children[3].queryAll(By.css('mat-icon'));
    expect(icons[0].nativeElement.innerText).toBe('lock_open');
    expect(servers.children[3].children[1].nativeElement.innerText).toContain('10.10.10.11:8000');

  });

  describe('RBAC', () => {
    beforeEach(() => {
      TestingUtility.removeAllPermissions();
    });

    it('should display arrows and rank', () => {
      TestingUtility.addPermissions([UIRolePermissions.authauthenticationpolicy_update, UIRolePermissions.authauthenticationpolicy_delete]);
      // if LDAPData is blank, it goes to create form which has no rank
      component.LDAPData = new AuthLdap({ enabled: true });
      fixture.detectChanges();
      const spy = spyOn(component.changeAuthRank, 'emit');
      // Current rank isn't set, shouldn't show anything
      let rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
      expect(rank.length).toBe(0);

      // NO ARROWS
      component.currentRank = 0;
      component.numRanks = 1;
      fixture.detectChanges();
      let arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(0);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(0);

      rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
      expect(rank.length).toBe(1);
      expect(rank[0].nativeElement.innerText).toContain('1');


      // UP ARROW ONLY
      component.currentRank = 1;
      component.numRanks = 2;
      fixture.detectChanges();
      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(0);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(1);
      arrow_container[0].children[0].nativeElement.click();
      expect(spy).toHaveBeenCalledTimes(1);
      expect(spy).toHaveBeenCalledWith(0);

      rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
      expect(rank.length).toBe(1);
      expect(rank[0].nativeElement.innerText).toContain('2');

      // DOWN ARROW ONLY
      spy.calls.reset();
      component.currentRank = 0;
      component.numRanks = 2;
      fixture.detectChanges();
      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(0);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(1);
      arrow_container[0].children[0].nativeElement.click();
      expect(spy).toHaveBeenCalledTimes(1);
      expect(spy).toHaveBeenCalledWith(1);

      rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
      expect(rank.length).toBe(1);
      expect(rank[0].nativeElement.innerText).toContain('1');

      // BOTH ARROWS
      component.currentRank = 1;
      component.numRanks = 3;
      fixture.detectChanges();
      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(1);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(1);

      rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
      expect(rank.length).toBe(1);
      expect(rank[0].nativeElement.innerText).toContain('2');

      // If we are in edit mode, arrows and rank are not visible
      // hover event
      fixture.debugElement.triggerEventHandler('mouseenter', null);
      fixture.detectChanges();
      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(3);
      arrow_container[0].children[0].nativeElement.click();
      fixture.detectChanges();

      // should be in edit mode
      const saveButton = fixture.debugElement.query(By.css('.authpolicy-save'));
      expect(saveButton).toBeTruthy();
      const cancelButton = fixture.debugElement.query(By.css('.authpolicy-cancel'));
      expect(cancelButton).toBeTruthy();

      // rank and arrows not visible
      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(0);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(0);

      rank = fixture.debugElement.queryAll(By.css('.authpolicy-rank'));
      expect(rank.length).toBe(0);
    });

    it('should create new', () => {
      TestingUtility.addPermissions([UIRolePermissions.authauthenticationpolicy_update, UIRolePermissions.authauthenticationpolicy_delete]);
      fixture.detectChanges();
      // overlay exists
      const overlay = fixture.debugElement.query(By.css('.ldap-overlay'));
      expect(overlay).toBeDefined();

      // create button exists
      const createButton = fixture.debugElement.query(By.css('.ldap-create'));
      expect(createButton).toBeDefined();

      // arrow and ranks should't exist
      let arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(0);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(1);
      expect(arrow_container[0].children.length).toBe(0);

      const rank = fixture.debugElement.query(By.css('.authpolicy-rank'));
      expect(rank).toBeNull();
      /*  //TODO: disable create LDAP in UI for GS-0.3
       // Clicking create button
       createButton.nativeElement.click();
       fixture.detectChanges();

       // should be in edit mode
       const saveButton = fixture.debugElement.query(By.css('.authpolicy-save'));
       expect(saveButton).toBeTruthy();
       const cancelButton = fixture.debugElement.query(By.css('.authpolicy-cancel'));
       expect(cancelButton).toBeTruthy();

       // Canceling the create form goes back to create new
       cancelButton.nativeElement.click();

       overlay = fixture.debugElement.query(By.css('.ldap-overlay'));
       expect(overlay).toBeDefined();
       createButton = fixture.debugElement.query(By.css('.ldap-create'));
       expect(createButton).toBeDefined();
       */

      // Saving the create form

    });

    it('edit mode', () => {
      TestingUtility.addPermissions([UIRolePermissions.authauthenticationpolicy_update, UIRolePermissions.authauthenticationpolicy_delete]);
      component.LDAPData = new AuthLdap({
        enabled: true,
        domains: [
          {
            'base-dn': 'basedn',
            'bind-dn': 'binddn',
            'bind-password': 'bindpass',
            'attribute-mapping': {
              'email': 'email',
              'fullname': 'fullname',
              'user': 'user',
              'user-object-class': 'user-obj',
              'group-object-class': 'group-obj-class',
              'group': 'group',
              'tenant': 'tenant'
            },
            servers: [
              {
                'url': '10.1.1.10:8000',
                'tls-options': {
                  'server-name': 'server1',
                  'skip-server-cert-verification': false,
                  'start-tls': true,
                  'trusted-certs': 'example cert'
                }
              },
              {
                'url': '10.1.1.11:8000',
                'tls-options': {
                  'server-name': 'server2',
                  'skip-server-cert-verification': true,
                  'start-tls': false,
                  'trusted-certs': 'example cert2'
                }
              }
            ]
          }
        ]
      });
      component.currentRank = 1;
      fixture.detectChanges();

      // edit button is only available on hover
      let arrow_container = fixture.debugElement.query(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.children.length).toBe(0);

      // hover event
      fixture.debugElement.triggerEventHandler('mouseenter', null);
      fixture.detectChanges();
      arrow_container = fixture.debugElement.query(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.children.length).toBe(2);
      arrow_container.children[0].nativeElement.click();
      fixture.detectChanges();

      // should be in edit mode
      const saveButton = fixture.debugElement.query(By.css('.authpolicy-save'));
      expect(saveButton).toBeTruthy();
      let cancelButton = fixture.debugElement.query(By.css('.authpolicy-cancel'));
      expect(cancelButton).toBeTruthy();

      let values = fixture.debugElement.queryAll(By.css('.ldap-input'));
      expect(values[0].nativeElement.value).toContain('binddn');
      expect(values[1].nativeElement.value).toContain('bindpass');
      expect(values[2].nativeElement.value).toContain('basedn');
      expect(values[3].nativeElement.value).toContain('user-obj');
      expect(values[4].nativeElement.value).toContain('group-obj-class');
      expect(values[5].nativeElement.value).toContain('user');
      expect(values[6].nativeElement.value).toContain('group');
      expect(values[7].nativeElement.value).toContain('tenant');
      expect(values[8].nativeElement.value).toContain('fullname');
      expect(values[9].nativeElement.value).toContain('email');
      expect(values[10].nativeElement.value).toContain('10.1.1.10:8000');
      expect(values[11].nativeElement.value).toContain('server1');
      expect(values[12].nativeElement.value).toContain('10.1.1.11:8000');
      expect(values[13].nativeElement.value).toContain('server2');

      let certs = fixture.debugElement.queryAll(By.css('.ldap-server-certs'));
      expect(certs.length).toBe(2);
      expect(certs[0].nativeElement.value).toBe('example cert');
      expect(certs[1].nativeElement.value).toBe('example cert2');

      // check toggles
      let toggles = fixture.debugElement.queryAll(By.css('.ldap-toggle'));
      const checkedToggles = fixture.debugElement.queryAll(By.css('.mat-checked'));
      const disabledToggles = fixture.debugElement.queryAll(By.css('.mat-disabled'));
      expect(toggles.length).toBe(4);
      expect(checkedToggles.length).toBe(2); // Enabled flag, and 2 of the server toggles
      expect(disabledToggles.length).toBe(1);

      // check disabled fields
      values = fixture.debugElement.queryAll(By.css('.ldap-input:disabled'));
      certs = fixture.debugElement.queryAll(By.css('.ldap-server-certs:disabled'));
      expect(values.length).toBe(1);
      expect(certs.length).toBe(1);

      // turning start-tls off should disable the other fields
      toggles[1].children[0].nativeElement.click();
      fixture.detectChanges();
      const checkedDisabledToggle = fixture.debugElement.queryAll(By.css('.mat-checked.mat-disabled'));
      expect(checkedDisabledToggle.length).toBe(0);

      values = fixture.debugElement.queryAll(By.css('.ldap-input:disabled'));
      certs = fixture.debugElement.queryAll(By.css('.ldap-server-certs:disabled'));
      expect(values.length).toBe(2);
      expect(certs.length).toBe(2);

      // checking add and delete of servers
      // Since there are two servers, there should be two trash cans and one and
      let and_container = fixture.debugElement.queryAll(By.css('.ldap-and'));
      expect(and_container.length).toBe(2);
      expect(and_container[0].children.length).toBe(2); // And and trash can
      expect(and_container[1].children.length).toBe(1); // trash can
      // deleting server
      and_container[1].children[0].nativeElement.click();
      fixture.detectChanges();
      let servers = fixture.debugElement.queryAll(By.css('.ldap-servergroup'));
      expect(servers.length).toBe(1);
      // add server
      and_container = fixture.debugElement.queryAll(By.css('.ldap-and'));
      expect(and_container.length).toBe(1);
      expect(and_container[0].children.length).toBe(1); // add only
      and_container[0].children[0].nativeElement.click();
      fixture.detectChanges();
      servers = fixture.debugElement.queryAll(By.css('.ldap-servergroup'));
      expect(servers.length).toBe(2);
      // new server should be inserted at position 0
      // and have start-tls and verify-cert enabled by default
      const serverToggles = servers[0].queryAll(By.css('.mat-checked'));
      expect(serverToggles.length).toBe(2);


      // checking changed values revert on cancel
      toggles[0].nativeElement.click();
      values = fixture.debugElement.queryAll(By.css('.ldap-input'));
      values[0].nativeElement.value = 'test';

      cancelButton = fixture.debugElement.query(By.css('.authpolicy-cancel'));
      cancelButton.nativeElement.click();
      fixture.detectChanges();

      toggles = fixture.debugElement.queryAll(By.css('.mat-checked'));
      expect(toggles.length).toBe(0);

      values = fixture.debugElement.queryAll(By.css('.ldap-input'));
      expect(values[0].nativeElement.innerText).toContain('binddn');

      const serversViewmode = fixture.debugElement.query(By.css('.ldap-server-viewmode'));
      expect(serversViewmode).toBeTruthy();
      expect(serversViewmode.children.length).toBe(4);

    });

    it('should add an empty server if there is none provided', () => {
      TestingUtility.addPermissions([UIRolePermissions.authauthenticationpolicy_update, UIRolePermissions.authauthenticationpolicy_delete]);
      component.LDAPData = new AuthLdap({
        enabled: true
      });
      fixture.detectChanges();

      // click edit which is only on hover
      fixture.debugElement.triggerEventHandler('mouseenter', null);
      fixture.detectChanges();
      const arrow_container = fixture.debugElement.query(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.children.length).toBe(2);
      arrow_container.children[0].nativeElement.click();
      fixture.detectChanges();

      const servers = fixture.debugElement.queryAll(By.css('.ldap-servergroup'));
      expect(servers.length).toBe(1);
    });

    it('should not create new', () => {
      TestingUtility.addPermissions([UIRolePermissions.authauthenticationpolicy_read]);
      fixture.detectChanges();

      // create button should not exist
      const createButton = fixture.debugElement.queryAll(By.css('.ldap-create'));
      expect(createButton.length).toBe(0);

      // arrows, edit and delete button should not exist
      // .authpolicy-arrow-up length 0 implies no edit, delete and arrow button are present
      let arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));
      expect(arrow_container.length).toBe(0);

      arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
      expect(arrow_container.length).toBe(0);
    });
  });
});
