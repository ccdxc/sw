import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import {NoopAnimationsModule} from '@angular/platform-browser/animations';
import {HttpClientTestingModule} from '@angular/common/http/testing';
import {PrimengModule} from '@lib/primeng.module';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import {RouterTestingModule} from '@angular/router/testing';
import {SharedModule} from '@components/shared/shared.module';
import { WidgetsModule } from 'web-app-framework';

import {ControllerService} from '@app/services/controller.service';
import {ConfirmationService} from 'primeng/api';
import {LogService} from '@app/services/logging/log.service';
import {LogPublishersService} from '@app/services/logging/log-publishers.service';
import {MatIconRegistry} from '@angular/material';
import {AuthService as AuthServiceGen} from '@app/services/generated/auth.service';
import {MessageService} from '@app/services/message.service';
import {StagingService} from '@app/services/generated/staging.service';
import {RolloutService} from '@app/services/generated/rollout.service';
import {ObjstoreService} from '@app/services/generated/objstore.service';

import { RolloutsComponent } from './rollouts.component';
import { NewrolloutComponent } from './newrollout/newrollout.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { AuthService } from '@app/services/auth.service';

describe('RolloutsComponent', () => {
  let component: RolloutsComponent;
  let fixture: ComponentFixture<RolloutsComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [ RolloutsComponent, NewrolloutComponent ],
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
        RolloutService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        AuthServiceGen,
        MessageService,
        StagingService,
        ObjstoreService
      ]
    });
    });

  beforeEach(() => {
    fixture = TestBed.createComponent(RolloutsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
