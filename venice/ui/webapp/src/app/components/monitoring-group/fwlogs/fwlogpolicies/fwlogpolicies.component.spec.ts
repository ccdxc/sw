import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { FwlogpoliciesComponent } from './fwlogpolicies.component';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { WidgetsModule } from 'web-app-framework';
import { PrimengModule } from '@app/lib/primeng.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { RouterTestingModule } from '@angular/router/testing';
import { SharedModule } from '@app/components/shared/shared.module';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { MessageService } from '@app/services/message.service';
import { MonitoringGroupModule } from '../../monitoring-group.module';
import { NewfwlogpolicyComponent } from './newfwlogpolicy/newfwlogpolicy.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { AuthService } from '@app/services/auth.service';
import { TestTablevieweditRBAC } from '@app/components/shared/tableviewedit/tableviewedit.component.spec';
import { MonitoringFwlogPolicy } from '@sdk/v1/models/generated/monitoring';
import { TestingUtility } from '@app/common/TestingUtility';


describe('FwlogpoliciesComponent', () => {
  let component: FwlogpoliciesComponent;
  let fixture: ComponentFixture<FwlogpoliciesComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [FwlogpoliciesComponent, NewfwlogpolicyComponent],
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
        MonitoringGroupModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        LicenseService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MonitoringService,
        MessageService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(FwlogpoliciesComponent);
    component = fixture.componentInstance;
  });

  it('should create', () => {
    fixture.detectChanges();
    expect(component).toBeTruthy();
  });

  describe('RBAC', () => {
    const testHelper = new TestTablevieweditRBAC('monitoringfwlogpolicy');

    beforeEach(() => {
      const service = TestBed.get(MonitoringService);
      spyOn(service, 'WatchFwlogPolicy').and.returnValue(
        TestingUtility.createWatchEventsSubject([new MonitoringFwlogPolicy()])
      );
      testHelper.fixture = fixture;
    });

    testHelper.runTests();
  });
});
