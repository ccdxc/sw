import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { NeweventalertpolicyComponent } from './neweventalertpolicy.component';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { RouterTestingModule } from '@angular/router/testing';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { PrimengModule } from '@app/lib/primeng.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { AuthService } from '@app/services/auth.service';
import { IMonitoringAlertPolicy, FieldsRequirement_operator, MonitoringAlertPolicySpec_severity, MonitoringAlertPolicy } from '@sdk/v1/models/generated/monitoring';
import { TestingUtility } from '@app/common/TestingUtility';
import * as _ from 'lodash';
import { WidgetsModule } from 'web-app-framework';
import { TrimUIFields } from '@sdk/v1/utils/utility';

describe('NeweventalertpolicyComponent', () => {
  let component: NeweventalertpolicyComponent;
  let fixture: ComponentFixture<NeweventalertpolicyComponent>;
  let tu: TestingUtility;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [NeweventalertpolicyComponent],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        SharedModule,
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
    fixture = TestBed.createComponent(NeweventalertpolicyComponent);
    tu = new TestingUtility(fixture);
    component = fixture.componentInstance;
  });

  it('should create', () => {
    const service = TestBed.get(MonitoringService);
    const spy = spyOn(service, 'AddAlertPolicy');
    fixture.detectChanges();
    const policy: IMonitoringAlertPolicy = {
      meta: {
        name: 'policy1'
      },
      spec: {
        enable: true,
        resource: 'Event',
        requirements: [
          {
            key: 'meta.name',
            operator: FieldsRequirement_operator.in,
            values: ['test1', 'test2']
          }
        ],
        severity: MonitoringAlertPolicySpec_severity.critical,
      }
    };
    tu.setInput('.neweventalertpolicy-name', policy.meta.name);
    tu.setRepeater(policy.spec.requirements);
    tu.setDropdown('.neweventalertpolicy-severity', policy.spec.severity);
    component.saveObject();
    fixture.detectChanges();
    expect(spy).toHaveBeenCalled();
    const recVal = spy.calls.mostRecent().args[0];
    const expVal = TrimUIFields(new MonitoringAlertPolicy(policy).getModelValues());
    expect(_.isEqual(recVal.meta, expVal.meta)).toBeTruthy('Received: ' + JSON.stringify(recVal) + ' , expected: ' + JSON.stringify(expVal));
  });

  it('should update', () => {
    component.isInline = true;
    const policy: IMonitoringAlertPolicy = {
      meta: {
        name: 'policy1'
      },
      spec: {
        enable: true,
        resource: 'Event',
        requirements: [
          {
            key: 'meta.name',
            operator: FieldsRequirement_operator.in,
            values: ['test1', 'test2']
          }
        ],
        severity: MonitoringAlertPolicySpec_severity.critical,
      }
    };
    component.objectData = policy;
    const service = TestBed.get(MonitoringService);
    const spy = spyOn(service, 'UpdateAlertPolicy');
    fixture.detectChanges();

    policy.spec.requirements[0].values = ['test3'];
    tu.setRepeater(policy.spec.requirements);

    tu.sendClick(tu.getElemByCss('.neweventalertpolicy-save'));
    expect(spy).toHaveBeenCalled();
    const recVal = spy.calls.mostRecent().args[1];
    const expVal = TrimUIFields(new MonitoringAlertPolicy(policy).getModelValues());
    expect(_.isEqual(recVal.meta, expVal.meta)).toBeTruthy('Received: ' + JSON.stringify(recVal.meta) + ' , expected: ' + JSON.stringify(expVal.meta));
    expect(_.isEqual(recVal.status, expVal.status)).toBeTruthy('Received: ' + JSON.stringify(recVal.status) + ' , expected: ' + JSON.stringify(expVal.status));
    expect(_.isEqual(recVal.spec.requirements, expVal.spec.requirements)).toBeTruthy('Received: ' + JSON.stringify(recVal.spec.requirements) + ' , expected: ' + JSON.stringify(expVal.spec.requirements));
  });
});
