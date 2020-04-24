import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { FlowexportpolicyComponent } from './flowexportpolicy.component';
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
import { TroubleshootGroupModule } from '../../troubleshoot-group.module';
import { NewflowexportpolicyComponent } from './newflowexportpolicy/newflowexportpolicy.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { AuthService } from '@app/services/auth.service';
import { TestTablevieweditRBAC } from '@app/components/shared/tableviewedit/tableviewedit.component.spec';
import { MonitoringFlowExportPolicy } from '@sdk/v1/models/generated/monitoring';


describe('FlowexportpolicyComponent', () => {
  let component: FlowexportpolicyComponent;
  let fixture: ComponentFixture<FlowexportpolicyComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [FlowexportpolicyComponent, NewflowexportpolicyComponent],
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
        TroubleshootGroupModule
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
    fixture = TestBed.createComponent(FlowexportpolicyComponent);
    component = fixture.componentInstance;
  });

  it('should create', () => {
    fixture.detectChanges();
    expect(component).toBeTruthy();
  });

  describe('RBAC', () => {
    const testHelper = new TestTablevieweditRBAC('monitoringflowexportpolicy');

    beforeEach(() => {
      component.dataObjects = [new MonitoringFlowExportPolicy()];
      testHelper.fixture = fixture;
    });

    testHelper.runTests();
  });
});
