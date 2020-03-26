import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { NeweventpolicyComponent } from './neweventpolicy.component';
import { MonitoringGroupModule } from '@app/components/monitoring-group/monitoring-group.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { ReactiveFormsModule, FormsModule } from '@angular/forms';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { MessageService } from '@app/services/message.service';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { AuthService } from '@app/services/auth.service';
import { TestingUtility } from '@app/common/TestingUtility';
import * as _ from 'lodash';
import { IMonitoringAlertPolicy, FieldsRequirement_operator, MonitoringAlertPolicySpec_severity, IMonitoringEventPolicy, MonitoringEventPolicySpec_format, MonitoringSyslogExportConfig_facility_override, MonitoringEventPolicy } from '@sdk/v1/models/generated/monitoring';
import { TrimUIFields } from '@sdk/v1/utils/utility';

describe('NeweventpolicyComponent', () => {
  let component: NeweventpolicyComponent;
  let fixture: ComponentFixture<NeweventpolicyComponent>;
  let tu: TestingUtility;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [NeweventpolicyComponent],
      imports: [
        MonitoringGroupModule,
        SharedModule,
        ReactiveFormsModule,
        RouterTestingModule,
        HttpClientTestingModule,
        FormsModule,
        NoopAnimationsModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
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
    fixture = TestBed.createComponent(NeweventpolicyComponent);
    component = fixture.componentInstance;
    tu = new TestingUtility(fixture);
  });

  it('should create', () => {
    const service = TestBed.get(MonitoringService);
    const spy = spyOn(service, 'AddEventPolicy');
    fixture.detectChanges();

    const policy: IMonitoringEventPolicy = {
      meta: {
        name: 'policy1'
      },
      spec: {
        format: MonitoringEventPolicySpec_format['syslog-rfc5424'],
        targets: [
          {
            destination: '1.1.1.1',
            gateway: null,
            transport: 'tcp/90'
          }
        ],
        config: {
          prefix: 'prefix',
          'facility-override': MonitoringSyslogExportConfig_facility_override.local0,
        }
      }
    };

    tu.setInput('.neweventpolicy-name', policy.meta.name);
    tu.setSyslogData(policy.spec as any);
    component.saveObject();
    fixture.detectChanges();
    expect(spy).toHaveBeenCalled();
    const recVal = spy.calls.mostRecent().args[0];
    const expVal = TrimUIFields(new MonitoringEventPolicy(policy).getModelValues());
    expect(_.isEqual(recVal, expVal)).toBeTruthy('Received: ' + recVal + ' , expected: ' + expVal);
  });

  it('should update', () => {
    component.isInline = true;
    const policy: IMonitoringEventPolicy = {
      meta: {
        name: 'policy1'
      },
      spec: {
        format: MonitoringEventPolicySpec_format['syslog-rfc5424'],
        targets: [
          {
            gateway: null,
            destination: '1.1.1.1',
            transport: 'tcp/90'
          }
        ],
        config: {
          prefix: 'prefix',
          'facility-override': MonitoringSyslogExportConfig_facility_override.local0,
        }
      }
    };
    component.objectData = policy;
    const service = TestBed.get(MonitoringService);
    const spy = spyOn(service, 'UpdateEventPolicy');
    fixture.detectChanges();

    policy.spec.targets[0].destination = '2.2.2.2';

    tu.setSyslogData(policy.spec as any);
    tu.sendClick(tu.getElemByCss('.global-button-primary.neweventpolicy-save'));
    expect(spy).toHaveBeenCalled();
    const recVal = spy.calls.mostRecent().args[1];
    const expVal = TrimUIFields(new MonitoringEventPolicy(policy).getModelValues());
    expect(_.isEqual(recVal, expVal)).toBeTruthy('Received: ' + JSON.stringify(recVal) + ' , expected: ' + JSON.stringify(expVal));
  });
});
