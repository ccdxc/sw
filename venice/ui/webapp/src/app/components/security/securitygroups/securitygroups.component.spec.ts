import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import {NoopAnimationsModule} from '@angular/platform-browser/animations';
import {HttpClientTestingModule} from '@angular/common/http/testing';
import {PrimengModule} from '@lib/primeng.module';
import {WidgetsModule} from 'web-app-framework';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import {RouterTestingModule} from '@angular/router/testing';
import {SharedModule} from '@components/shared/shared.module';
import {ControllerService} from '@app/services/controller.service';
import {ConfirmationService} from 'primeng/api';
import {LogService} from '@app/services/logging/log.service';
import {LogPublishersService} from '@app/services/logging/log-publishers.service';
import {MatIconRegistry} from '@angular/material';
import {MessageService} from '@app/services/message.service';
import { ISecuritySecurityGroup, SecuritySecurityGroup} from '@sdk/v1/models/generated/security/security-security-group.model';
import { SecurityService } from '@app/services/generated/security.service';
import { AuthService } from '@app/services/auth.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { WorkloadService } from '@app/services/generated/workload.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';

import { NewsecuritygroupComponent } from './newsecuritygroup/newsecuritygroup.component';
import { SecuritygroupsComponent } from './securitygroups.component';

describe('SecuritygroupsComponent', () => {
  let component: SecuritygroupsComponent;
  let fixture: ComponentFixture<SecuritygroupsComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [ SecuritygroupsComponent, NewsecuritygroupComponent ],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        PrimengModule,
        WidgetsModule,
        MaterialdesignModule,
        RouterTestingModule,
        SharedModule,
        ],
      providers: [
        ControllerService,
        UIConfigsService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        SecurityService,
        MessageService,
        WorkloadService
      ]
    })
    .compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(SecuritygroupsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
