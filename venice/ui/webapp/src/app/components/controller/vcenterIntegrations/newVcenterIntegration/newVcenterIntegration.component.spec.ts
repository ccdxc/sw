import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { NewVcenterIntegrationComponent } from './newVcenterIntegration.component';
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
import {OrchestrationService} from '@app/services/generated/orchestration.service';
import { AuthService } from '@app/services/auth.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { StagingService } from '@app/services/generated/staging.service';

describe('NewVcenterIntegrationComponent', () => {
  let component: NewVcenterIntegrationComponent;
  let fixture: ComponentFixture<NewVcenterIntegrationComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
     declarations: [NewVcenterIntegrationComponent],
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
       OrchestrationService,
       MessageService,
       StagingService
     ]
   });
  });

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NewVcenterIntegrationComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NewVcenterIntegrationComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
