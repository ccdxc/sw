import {  ComponentFixture, TestBed, async } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { AuthpolicyComponent } from './authpolicy.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { LdapComponent } from '@app/components/admin/authpolicy/ldap/ldap.component';
import { RadiusComponent } from '@app/components/admin/authpolicy/radius/radius.component';
import { LocalComponent } from '@app/components/admin/authpolicy/local/local.component';
import { SharedModule } from '@app/components/shared/shared.module';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { AuthService as AuthServiceGen } from '@app/services/generated/auth.service';
import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { Observable, of } from 'rxjs';
import { AuthAuthenticators_authenticator_order } from '@sdk/v1/models/generated/auth';
import { By } from '@angular/platform-browser';
import { MessageService } from '@app/services/message.service';
import { PrimengModule } from '@app/lib/primeng.module';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { AuthService } from '@app/services/auth.service';
import { TestingUtility } from '@app/common/TestingUtility';

class MockAuthService extends AuthService {
  public GetAuthenticationPolicy(): any {
    const items = {
      spec: {
        authenticators: {
          'authenticator-order': [AuthAuthenticators_authenticator_order.local, AuthAuthenticators_authenticator_order.ldap],
          ldap: {
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
          },
          local: {
            enabled: true
          }

        }
      }
    };
    const response = { body: items };
    return of(response);
  }
}

describe('AuthpolicyComponent', () => {
  let component: AuthpolicyComponent;
  let fixture: ComponentFixture<AuthpolicyComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [
        AuthpolicyComponent,
        LdapComponent,
        RadiusComponent,
        LocalComponent
      ],
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
        UIConfigsService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MessageService,
        { provide: AuthServiceGen, useClass: MockAuthService },
        MatIconRegistry,
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(AuthpolicyComponent);
    component = fixture.componentInstance;
  });

  it('should create', () => {
    fixture.detectChanges();
    expect(component).toBeTruthy();
  });

  describe('RBAC', () => {

    it('should fetch data and change ranks', async(() => {
      // TODO: Add RADIUS data
      TestingUtility.addPermissions([UIRolePermissions.authauthenticationpolicy_update]);
      const authService = fixture.debugElement.injector.get(AuthServiceGen);
      const spy = spyOn(authService, 'GetAuthenticationPolicy').and.callThrough();
      fixture.detectChanges();
      expect(spy).toHaveBeenCalled();
      fixture.whenRenderingDone().then(() => {
        let arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-up'));

        // moving second item up a rank
        arrow_container[1].children[0].nativeElement.click();
        fixture.detectChanges();
        fixture.whenRenderingDone().then(() => {
          // Items should be switched now
          let groups = fixture.debugElement.queryAll(By.css('.authpolicy-group'));
          let ldap = groups[0].query(By.css('.ldap-title'));
          expect(ldap).toBeTruthy();
          let local = groups[1].query(By.css('.local-group-title'));
          expect(local).toBeTruthy();
          let radius = groups[2].query(By.css('.radius-title'));
          expect(radius).toBeTruthy();
          // checking that the data is the same
          const toggle = groups[0].queryAll(By.css('.mat-checked'));
          expect(toggle.length).toBe(0);
          const values = groups[0].queryAll(By.css('.ldap-input'));
          expect(values[0].nativeElement.innerHTML).toContain('binddn');
          expect(values[1].nativeElement.innerHTML).toContain('**********'); // VS-214 use password input
          expect(values[2].nativeElement.innerHTML).toContain('basedn');
          expect(values[3].nativeElement.innerHTML).toContain('user-obj');
          expect(values[4].nativeElement.innerHTML).toContain('group-obj-class');
          expect(values[5].nativeElement.innerHTML).toContain('user');
          expect(values[6].nativeElement.innerHTML).toContain('group');
          expect(values[7].nativeElement.innerHTML).toContain('tenant');
          expect(values[8].nativeElement.innerHTML).toContain('fullname');
          expect(values[9].nativeElement.innerHTML).toContain('email');

          // moving it back down
          arrow_container = fixture.debugElement.queryAll(By.css('.authpolicy-arrow-down'));
          arrow_container[0].children[0].nativeElement.click();
          fixture.detectChanges();

          // Items should be switched now
          groups = fixture.debugElement.queryAll(By.css('.authpolicy-group'));
          local = groups[0].query(By.css('.local-group-title'));
          expect(local).toBeTruthy();
          ldap = groups[1].query(By.css('.ldap-title'));
          expect(ldap).toBeTruthy();
          radius = groups[2].query(By.css('.radius-title'));
          expect(radius).toBeTruthy();
        });
      });
    }));
  });

});
