import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { NewfwlogpolicyComponent } from './newfwlogpolicy.component';
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
import { PrimengModule } from '@app/lib/primeng.module';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { AuthService } from '@app/services/auth.service';
import { IMonitoringFwlogPolicy, MonitoringEventPolicySpec_format, MonitoringFwlogPolicySpec_filter, MonitoringFwlogPolicySpec_format, MonitoringSyslogExportConfig_facility_override, MonitoringFwlogPolicy } from '@sdk/v1/models/generated/monitoring';
import { TestingUtility } from '@app/common/TestingUtility';
import * as _ from 'lodash';
import { TrimUIFields } from '@sdk/v1/utils/utility';


describe('NewfwlogpolicyComponent', () => {
  let component: NewfwlogpolicyComponent;
  let fixture: ComponentFixture<NewfwlogpolicyComponent>;
  let tu: TestingUtility;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [NewfwlogpolicyComponent],
      imports: [
        MonitoringGroupModule,
        SharedModule,
        ReactiveFormsModule,
        RouterTestingModule,
        HttpClientTestingModule,
        FormsModule,
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
        MatIconRegistry,
        MonitoringService,
        MessageService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(NewfwlogpolicyComponent);
    tu = new TestingUtility(fixture);
    component = fixture.componentInstance;
  });

  it('should create', () => {
    const service = TestBed.get(MonitoringService);
    const spy = spyOn(service, 'AddFwlogPolicy');
    fixture.detectChanges();

    const policy: IMonitoringFwlogPolicy = {
      meta: {
        name: 'policy1'
      },
      spec: {
        filter: [MonitoringFwlogPolicySpec_filter.all],
        format: MonitoringFwlogPolicySpec_format['syslog-rfc5424'],
        targets: [
          {
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

    tu.setInput('.newfwlogpolicy-name', policy.meta.name);
    tu.setSyslogData(policy.spec as any);
    component.saveObject();
    fixture.detectChanges();
    expect(spy).toHaveBeenCalled();
    const recVal = spy.calls.mostRecent().args[0];
    const expVal = TrimUIFields(new MonitoringFwlogPolicy(policy).getModelValues());
    expect(_.isEqual(recVal, expVal)).toBeTruthy('Received: ' + recVal + ' , expected: ' + expVal);
  });

  it('should update', () => {
    component.isInline = true;
    const policy: IMonitoringFwlogPolicy = {
      meta: {
        name: 'policy1'
      },
      spec: {
        filter: [MonitoringFwlogPolicySpec_filter.all],
        format: MonitoringFwlogPolicySpec_format['syslog-rfc5424'],
        targets: [
          {
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
    const spy = spyOn(service, 'UpdateFwlogPolicy');
    fixture.detectChanges();

    policy.spec.targets[0].destination = '2.2.2.2';

    tu.setSyslogData(policy.spec as any);
    tu.sendClick(tu.getElemByCss('.global-button-primary.newfwlogpolicy-save'));
    expect(spy).toHaveBeenCalled();
    const recVal = spy.calls.mostRecent().args[1];
    const expVal = TrimUIFields(new MonitoringFwlogPolicy(policy).getModelValues());
    expect(_.isEqual(recVal, expVal)).toBeTruthy('Received: ' + JSON.stringify(recVal) + ' , expected: ' + JSON.stringify(expVal));
  });
});
