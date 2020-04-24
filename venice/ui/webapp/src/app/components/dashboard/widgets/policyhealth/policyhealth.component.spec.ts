import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { PrimengModule } from '@app/lib/primeng.module';

import { PolicyhealthComponent } from './policyhealth.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { SharedModule } from '@app/components/shared/shared.module';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { ControllerService } from '@app/services/controller.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from '@app/services/message.service';
import { LogService } from '@app/services/logging/log.service';
import { ConfirmationService } from 'primeng/api';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { AuthService } from '@app/services/auth.service';

describe('PolicyhealthComponent', () => {
  let component: PolicyhealthComponent;
  let fixture: ComponentFixture<PolicyhealthComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [PolicyhealthComponent],
      imports: [
        PrimengModule,
        RouterTestingModule,
        HttpClientTestingModule,
        MaterialdesignModule,
        NoopAnimationsModule,
        SharedModule
      ],
      providers: [
        MetricsqueryService,
        ControllerService,
        LogService,
        LogPublishersService,
        MessageService,
        ConfirmationService,
        UIConfigsService,
        LicenseService,
        AuthService
      ]
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(PolicyhealthComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
