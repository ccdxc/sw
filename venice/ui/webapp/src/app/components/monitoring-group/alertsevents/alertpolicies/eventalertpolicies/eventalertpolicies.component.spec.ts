/**-----
 Angular imports
 ------------------*/
import { HttpClientTestingModule } from '@angular/common/http/testing';
import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { SharedModule } from '@app/components/shared/shared.module';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { EventalertpolicyComponent } from './eventalertpolicies.component';
import { NeweventalertpolicyComponent } from '@app/components/monitoring-group/alertsevents/alertpolicies/eventalertpolicies/neweventalertpolicy/neweventalertpolicy.component';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { WidgetsModule } from 'web-app-framework';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { TestingUtility } from '@app/common/TestingUtility';
import { AuthService } from '@app/services/auth.service';
import { MonitoringAlertPolicy } from '@sdk/v1/models/generated/monitoring';
import { TestTablevieweditRBAC } from '@app/components/shared/tableviewedit/tableviewedit.component.spec';

describe('EventalertpolicyComponent', () => {
  let component: EventalertpolicyComponent;
  let fixture: ComponentFixture<EventalertpolicyComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [EventalertpolicyComponent, NeweventalertpolicyComponent],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        WidgetsModule
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
    fixture = TestBed.createComponent(EventalertpolicyComponent);
    component = fixture.componentInstance;
  });

  it('should create', () => {
    fixture.detectChanges();
    expect(component).toBeTruthy();
  });

  describe('RBAC', () => {
    const testHelper = new TestTablevieweditRBAC('monitoringalertpolicy');

    beforeEach(() => {
      component.isActiveTab = true;
      component.dataObjects = [new MonitoringAlertPolicy().getModelValues()];
      testHelper.fixture = fixture;
    });

    testHelper.runTests();
  });

  describe('RBAC', () => {
    const testHelper = new TestTablevieweditRBAC('monitoringalertpolicy');

    beforeEach(() => {
      component.isActiveTab = true;
      component.dataObjects = [new MonitoringAlertPolicy()];
      testHelper.fixture = fixture;
    });

    testHelper.runTests();
  });

});
